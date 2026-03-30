import re
from typing import Annotated

from pydantic import BaseModel, Field, field_validator

from config import settings

MAX_DEPTH = settings.max_search_depth
DEFAULT_DEPTH = settings.default_search_depth

# ---------------------------------------------------------------------------
# FEN validation
# ---------------------------------------------------------------------------

# Piece-placement field: 8 ranks separated by '/', each rank composed of
# piece letters (pnbrqkPNBRQK) and digit run-lengths (1-8).
_RANK_RE = re.compile(r"^[pnbrqkPNBRQK1-8]{1,8}$")
_CASTLING_RE = re.compile(r"^(-|[KQkq]{1,4})$")
_EN_PASSANT_RE = re.compile(r"^(-|[a-h][36])$")


def _validate_fen(fen: str) -> str:
    """Raise ValueError with a human-readable message for any invalid FEN."""
    fen = fen.strip()
    if not fen:
        raise ValueError("FEN must not be empty")

    parts = fen.split()
    if len(parts) != 6:
        raise ValueError(
            f"FEN must have 6 space-separated fields, got {len(parts)}"
        )

    placement, side, castling, ep, halfmove, fullmove = parts

    # --- Piece placement ---
    ranks = placement.split("/")
    if len(ranks) != 8:
        raise ValueError(
            f"Piece placement must have 8 ranks separated by '/', got {len(ranks)}"
        )
    for i, rank in enumerate(ranks):
        if not _RANK_RE.match(rank):
            raise ValueError(f"Invalid characters in rank {8 - i}: '{rank}'")
        squares = sum(int(c) if c.isdigit() else 1 for c in rank)
        if squares != 8:
            raise ValueError(
                f"Rank {8 - i} covers {squares} squares, expected 8"
            )

    # --- Side to move ---
    if side not in ("w", "b"):
        raise ValueError(f"Side to move must be 'w' or 'b', got '{side}'")

    # --- Castling ---
    if not _CASTLING_RE.match(castling):
        raise ValueError(f"Invalid castling field: '{castling}'")

    # --- En passant ---
    if not _EN_PASSANT_RE.match(ep):
        raise ValueError(f"Invalid en-passant field: '{ep}'")

    # --- Half-move clock ---
    if not halfmove.isdigit():
        raise ValueError(f"Half-move clock must be a non-negative integer, got '{halfmove}'")

    # --- Full-move number ---
    if not fullmove.isdigit() or int(fullmove) < 1:
        raise ValueError(f"Full-move number must be a positive integer, got '{fullmove}'")

    # --- King counts (must have exactly one of each) ---
    if placement.count("K") != 1 or placement.count("k") != 1:
        raise ValueError("Position must contain exactly one white king and one black king")

    return fen


# ---------------------------------------------------------------------------
# Request models
# ---------------------------------------------------------------------------


class FENRequest(BaseModel):
    fen: str = Field(..., description="Position in FEN notation")

    @field_validator("fen")
    @classmethod
    def fen_must_be_valid(cls, v: str) -> str:
        return _validate_fen(v)


class SearchRequest(BaseModel):
    fen: str = Field(..., description="Position in FEN notation")
    depth: Annotated[int, Field(ge=1, le=MAX_DEPTH)] = Field(
        DEFAULT_DEPTH, description=f"Search depth (1–{MAX_DEPTH})"
    )
    time_limit_ms: Annotated[int, Field(ge=100, le=settings.max_time_limit_ms)] = Field(
        settings.default_time_limit_ms,
        description=f"Search time limit in milliseconds (100–{settings.max_time_limit_ms})",
    )

    @field_validator("fen")
    @classmethod
    def fen_must_be_valid(cls, v: str) -> str:
        return _validate_fen(v)


# ---------------------------------------------------------------------------
# Response models
# ---------------------------------------------------------------------------


class LegalMovesResponse(BaseModel):
    fen: str
    moves: list[str] = Field(..., description="Legal moves in UCI notation (e.g. e2e4)")
    count: int


class EvaluateResponse(BaseModel):
    fen: str
    score_cp: int = Field(
        ..., description="Static evaluation in centipawns, relative to side to move"
    )


class SearchResponse(BaseModel):
    fen: str
    depth: int
    score_cp: int | None = Field(None, description="Score in centipawns (side to move)")
    score_mate: int | None = Field(
        None, description="Forced mate in N (negative = being mated)"
    )
    best_move: str
    pv: list[str] = Field(
        default_factory=list, description="Principal variation in UCI notation"
    )
    nodes: int
    elapsed_ms: int = Field(0, description="Search time in milliseconds")
    nps: int = Field(0, description="Nodes per second")
