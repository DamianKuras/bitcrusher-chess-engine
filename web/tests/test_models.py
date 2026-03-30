"""Unit tests for request/response model validation."""

import pytest
from pydantic import ValidationError

from models import FENRequest, SearchRequest

VALID_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
KIWIPETE = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"


# ---------------------------------------------------------------------------
# FENRequest
# ---------------------------------------------------------------------------

class TestFENRequest:
    def test_valid_starting_position(self):
        req = FENRequest(fen=VALID_FEN)
        assert req.fen == VALID_FEN

    def test_valid_kiwipete(self):
        req = FENRequest(fen=KIWIPETE)
        assert req.fen == KIWIPETE

    def test_strips_surrounding_whitespace(self):
        req = FENRequest(fen=f"  {VALID_FEN}  ")
        assert req.fen == VALID_FEN

    def test_empty_fen_rejected(self):
        with pytest.raises(ValidationError, match="empty"):
            FENRequest(fen="")

    def test_too_few_fields(self):
        with pytest.raises(ValidationError, match="6 space-separated fields"):
            FENRequest(fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -")

    def test_wrong_rank_count(self):
        with pytest.raises(ValidationError, match="8 ranks"):
            FENRequest(fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1")

    def test_rank_wrong_square_count(self):
        # "9" is an invalid rank - would be 9 squares
        with pytest.raises(ValidationError):
            FENRequest(fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPP1P w KQkq - 0 1")

    def test_invalid_side_to_move(self):
        with pytest.raises(ValidationError, match="Side to move"):
            FENRequest(fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1")

    def test_invalid_castling(self):
        with pytest.raises(ValidationError, match="castling"):
            FENRequest(fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w XYZ - 0 1")

    def test_invalid_en_passant(self):
        with pytest.raises(ValidationError, match="en-passant"):
            FENRequest(fen="rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e5 0 1")

    def test_missing_white_king(self):
        # Replace white king with a pawn
        bad = VALID_FEN.replace("RNBQKBNR", "RNBQPBNR")
        with pytest.raises(ValidationError, match="white king"):
            FENRequest(fen=bad)

    def test_missing_black_king(self):
        bad = VALID_FEN.replace("rnbqkbnr", "rnbqpbnr")
        with pytest.raises(ValidationError, match="black king"):
            FENRequest(fen=bad)

    def test_non_positive_fullmove(self):
        with pytest.raises(ValidationError, match="Full-move"):
            FENRequest(fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0")

    def test_non_numeric_halfmove(self):
        with pytest.raises(ValidationError, match="Half-move"):
            FENRequest(fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - x 1")


# ---------------------------------------------------------------------------
# SearchRequest
# ---------------------------------------------------------------------------

class TestSearchRequest:
    def test_defaults(self):
        req = SearchRequest(fen=VALID_FEN)
        assert req.depth >= 1
        assert req.time_limit_ms >= 100

    def test_depth_too_low(self):
        with pytest.raises(ValidationError):
            SearchRequest(fen=VALID_FEN, depth=0)

    def test_depth_at_max(self):
        from config import settings
        req = SearchRequest(fen=VALID_FEN, depth=settings.max_search_depth)
        assert req.depth == settings.max_search_depth

    def test_depth_above_max(self):
        from config import settings
        with pytest.raises(ValidationError):
            SearchRequest(fen=VALID_FEN, depth=settings.max_search_depth + 1)

    def test_time_limit_below_minimum(self):
        with pytest.raises(ValidationError):
            SearchRequest(fen=VALID_FEN, time_limit_ms=50)

    def test_time_limit_above_max(self):
        from config import settings
        with pytest.raises(ValidationError):
            SearchRequest(fen=VALID_FEN, time_limit_ms=settings.max_time_limit_ms + 1)

    def test_fen_validated_in_search_request(self):
        with pytest.raises(ValidationError):
            SearchRequest(fen="not a fen", depth=5)
