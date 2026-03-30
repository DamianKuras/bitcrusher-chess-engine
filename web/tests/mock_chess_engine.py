"""Fake chess_engine module - stands in for the pybind11 .so in tests.

Installed into sys.modules by conftest.py before any import of the real module.
"""

STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

_LEGAL_MOVES = [
    "a2a3", "a2a4", "b2b3", "b2b4", "c2c3", "c2c4",
    "d2d3", "d2d4", "e2e3", "e2e4", "f2f3", "f2f4",
    "g2g3", "g2g4", "h2h3", "h2h4",
    "b1a3", "b1c3", "g1f3", "g1h3",
]


def legal_moves(fen: str) -> list[str]:
    return list(_LEGAL_MOVES)


def evaluate(fen: str) -> int:
    return 15


def search(fen: str, depth: int = 12, time_limit_ms: int = 10000) -> dict:
    return {
        "best_move": "e2e4",
        "pv": ["e2e4", "e7e5"],
        "nodes": 12345,
        "score_cp": 30,
        "score_mate": None,
    }
