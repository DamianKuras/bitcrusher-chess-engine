"""Integration tests for all API endpoints."""

import pytest
from fastapi.testclient import TestClient

VALID_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
KIWIPETE = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"


# ---------------------------------------------------------------------------
# /health
# ---------------------------------------------------------------------------

class TestHealth:
    def test_ok(self, client: TestClient):
        resp = client.get("/health")
        assert resp.status_code == 200
        assert resp.json()["status"] == "ok"


# ---------------------------------------------------------------------------
# /legal-moves
# ---------------------------------------------------------------------------

class TestLegalMoves:
    def test_starting_position(self, client: TestClient):
        resp = client.post("/legal-moves", json={"fen": VALID_FEN})
        assert resp.status_code == 200
        body = resp.json()
        assert body["fen"] == VALID_FEN
        assert isinstance(body["moves"], list)
        assert body["count"] == len(body["moves"])
        assert len(body["moves"]) > 0

    def test_kiwipete(self, client: TestClient):
        resp = client.post("/legal-moves", json={"fen": KIWIPETE})
        assert resp.status_code == 200
        assert resp.json()["count"] > 0

    def test_invalid_fen_returns_422(self, client: TestClient):
        resp = client.post("/legal-moves", json={"fen": "not a valid fen"})
        assert resp.status_code == 422

    def test_missing_fen_field_returns_422(self, client: TestClient):
        resp = client.post("/legal-moves", json={})
        assert resp.status_code == 422

    def test_empty_fen_returns_422(self, client: TestClient):
        resp = client.post("/legal-moves", json={"fen": ""})
        assert resp.status_code == 422

    def test_wrong_rank_count_returns_422(self, client: TestClient):
        bad = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1"
        resp = client.post("/legal-moves", json={"fen": bad})
        assert resp.status_code == 422


# ---------------------------------------------------------------------------
# /evaluate
# ---------------------------------------------------------------------------

class TestEvaluate:
    def test_starting_position(self, client: TestClient):
        resp = client.post("/evaluate", json={"fen": VALID_FEN})
        assert resp.status_code == 200
        body = resp.json()
        assert body["fen"] == VALID_FEN
        assert isinstance(body["score_cp"], int)

    def test_invalid_fen_returns_422(self, client: TestClient):
        resp = client.post("/evaluate", json={"fen": "garbage"})
        assert resp.status_code == 422

    def test_missing_fen_returns_422(self, client: TestClient):
        resp = client.post("/evaluate", json={})
        assert resp.status_code == 422


# ---------------------------------------------------------------------------
# /search
# ---------------------------------------------------------------------------

class TestSearch:
    def test_basic_search(self, client: TestClient):
        resp = client.post("/search", json={"fen": VALID_FEN, "depth": 3})
        assert resp.status_code == 200
        body = resp.json()
        assert body["fen"] == VALID_FEN
        assert body["depth"] == 3
        assert isinstance(body["best_move"], str)
        assert len(body["best_move"]) >= 4
        assert isinstance(body["nodes"], int)
        assert body["nodes"] >= 0
        # Exactly one of score_cp / score_mate must be non-null.
        assert (body["score_cp"] is not None) or (body["score_mate"] is not None)

    def test_default_depth_used(self, client: TestClient):
        from config import settings
        resp = client.post("/search", json={"fen": VALID_FEN})
        assert resp.status_code == 200
        assert resp.json()["depth"] == settings.default_search_depth

    def test_custom_time_limit(self, client: TestClient):
        resp = client.post("/search", json={"fen": VALID_FEN, "depth": 3, "time_limit_ms": 500})
        assert resp.status_code == 200

    def test_depth_zero_rejected(self, client: TestClient):
        resp = client.post("/search", json={"fen": VALID_FEN, "depth": 0})
        assert resp.status_code == 422

    def test_depth_above_max_rejected(self, client: TestClient):
        from config import settings
        resp = client.post("/search", json={"fen": VALID_FEN, "depth": settings.max_search_depth + 1})
        assert resp.status_code == 422

    def test_time_limit_too_low_rejected(self, client: TestClient):
        resp = client.post("/search", json={"fen": VALID_FEN, "time_limit_ms": 10})
        assert resp.status_code == 422

    def test_invalid_fen_returns_422(self, client: TestClient):
        resp = client.post("/search", json={"fen": "invalid", "depth": 3})
        assert resp.status_code == 422

    def test_missing_fen_returns_422(self, client: TestClient):
        resp = client.post("/search", json={"depth": 3})
        assert resp.status_code == 422

    def test_pv_is_list(self, client: TestClient):
        resp = client.post("/search", json={"fen": VALID_FEN, "depth": 3})
        assert isinstance(resp.json()["pv"], list)

    def test_elapsed_ms_and_nps_present(self, client: TestClient):
        resp = client.post("/search", json={"fen": VALID_FEN, "depth": 3})
        body = resp.json()
        assert "elapsed_ms" in body
        assert "nps" in body


# ---------------------------------------------------------------------------
# Engine error propagation
# ---------------------------------------------------------------------------

class TestEngineErrorPropagation:
    def test_engine_value_error_returns_422(self, client: TestClient, monkeypatch):
        import engine

        async def boom(fen: str) -> list[str]:
            from fastapi import HTTPException
            raise HTTPException(status_code=422, detail="bad fen from engine")

        monkeypatch.setattr(engine, "get_legal_moves", boom)
        resp = client.post("/legal-moves", json={"fen": VALID_FEN})
        assert resp.status_code == 422

    def test_engine_internal_error_returns_500(self, client: TestClient, monkeypatch):
        import engine

        async def boom(fen: str) -> int:
            from fastapi import HTTPException
            raise HTTPException(status_code=500, detail="Engine error computing evaluation")

        monkeypatch.setattr(engine, "get_evaluate", boom)
        resp = client.post("/evaluate", json={"fen": VALID_FEN})
        assert resp.status_code == 500
