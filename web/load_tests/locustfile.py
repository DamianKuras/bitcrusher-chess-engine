"""
Bitcrusher Chess API - Locust load test.

Run:
    cd load_tests
    uv run locust                          # opens web UI at http://localhost:8089
    uv run locust --headless -u 20 -r 5   # headless, 20 users, ramp 5/s

Separate user classes let you target individual endpoints:
    uv run locust LegalMovesUser
    uv run locust EvaluateUser
    uv run locust SearchUser
    uv run locust MixedUser               # realistic mix of all three
"""

import random

from locust import HttpUser, between, task

# A variety of positions to avoid caching effects - opening, middlegame, endgame.
FENS = [
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
]

SEARCH_DEPTHS = [3, 4, 5]


class LegalMovesUser(HttpUser):
    """Hammers the fast /legal-moves endpoint."""

    host = "http://localhost:8000"
    wait_time = between(0.05, 0.2)

    @task
    def legal_moves(self):
        self.client.post("/legal-moves", json={"fen": random.choice(FENS)})


class EvaluateUser(HttpUser):
    """Hammers the fast /evaluate endpoint."""

    host = "http://localhost:8000"
    wait_time = between(0.05, 0.2)

    @task
    def evaluate(self):
        self.client.post("/evaluate", json={"fen": random.choice(FENS)})


class SearchUser(HttpUser):
    """Tests the CPU-heavy /search endpoint at low depth."""

    host = "http://localhost:8000"
    wait_time = between(0.5, 2.0)

    @task
    def search(self):
        self.client.post(
            "/search",
            json={"fen": random.choice(FENS), "depth": random.choice(SEARCH_DEPTHS)},
        )


class MixedUser(HttpUser):
    """
    Realistic traffic mix:
      - legal-moves  50 %  (e.g. frontend rendering move hints)
      - evaluate     30 %  (e.g. position bar)
      - search        20 %  (e.g. engine move request)
    """

    host = "http://localhost:8000"
    wait_time = between(0.1, 0.5)

    @task(5)
    def legal_moves(self):
        self.client.post("/legal-moves", json={"fen": random.choice(FENS)})

    @task(3)
    def evaluate(self):
        self.client.post("/evaluate", json={"fen": random.choice(FENS)})

    @task(2)
    def search(self):
        self.client.post(
            "/search",
            json={"fen": random.choice(FENS), "depth": random.choice(SEARCH_DEPTHS)},
        )
