# Bitcrusher Web

Full-stack chess web app - React frontend + FastAPI backend powered by the Bitcrusher engine running in-process via pybind11.

## What's inside

```
web/
├── frontend/        React + Vite + Tailwind chess UI
├── bindings/        pybind11 C++ → Python extension
├── main.py          FastAPI app (routes, middleware, lifecycle)
├── engine.py        Async wrapper around the C++ extension
├── models.py        Request/response models + FEN validation
├── config.py        All settings, read from env vars
├── tests/           pytest suite (no C++ build required)
├── load_tests/      Locust load test scenarios
├── Dockerfile       Two-stage build (C++ → Python runtime)
└── docker-compose.yml
```

---

## Running the app

```bash
# From the repo root
docker compose -f web/docker-compose.yml up --build
```

| URL                            | What you get                          |
| ------------------------------ | ------------------------------------- |
| `http://localhost`             | Chess app - Play tab + Analysis tab   |
| `http://localhost:8000/docs`   | Swagger UI / interactive API explorer |
| `http://localhost:8000/health` | Liveness check                        |

Two containers start: `api` (FastAPI + engine) and `frontend` (Nginx serving the React build). The frontend is the main entry point.

### Frontend hot-reload (dev mode)

```bash
# Terminal 1 - API only
docker compose -f web/docker-compose.yml up api

# Terminal 2 - Vite dev server (proxies /api calls automatically)
cd web/frontend
npm install
npm run dev    # → http://localhost:5173
```

---

## Configuration

All settings use the `BITCRUSHER_` prefix and can be set as environment variables or in an `web/.env` file.

| Variable                             | Default                                             | Description                       |
| ------------------------------------ | --------------------------------------------------- | --------------------------------- |
| `BITCRUSHER_CORS_ORIGINS`            | `["http://localhost:5173","http://localhost:3000"]` | Allowed CORS origins              |
| `BITCRUSHER_MAX_CONCURRENT_SEARCHES` | `0` (= `cpu_count`)                                 | Max parallel search calls         |
| `BITCRUSHER_LEGAL_MOVES_CACHE_SIZE`  | `4096`                                              | LRU cache size for `/legal-moves` |
| `BITCRUSHER_EVALUATE_CACHE_SIZE`     | `4096`                                              | LRU cache size for `/evaluate`    |
| `BITCRUSHER_DEFAULT_SEARCH_DEPTH`    | `12`                                                | Depth when omitted from request   |
| `BITCRUSHER_MAX_SEARCH_DEPTH`        | `50`                                                | Maximum accepted depth            |
| `BITCRUSHER_DEFAULT_TIME_LIMIT_MS`   | `10000`                                             | Default search time cap (ms)      |
| `BITCRUSHER_MAX_TIME_LIMIT_MS`       | `60000`                                             | Maximum accepted time limit (ms)  |
| `BITCRUSHER_RATE_LIMIT_PER_MINUTE`   | `60`                                                | Per-IP rate limit (0 = disabled)  |

---

## API reference

### `GET /health`

Returns `200 {"status": "ok"}` when the engine is ready, `503` otherwise.

### `POST /legal-moves`

```json
// Request
{"fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"}

// Response
{"fen": "...", "moves": ["a2a3", "a2a4", ...], "count": 20}
```

Results are LRU-cached - repeated calls with the same FEN are instant.

### `POST /evaluate`

Static evaluation in centipawns relative to the side to move (no search).

```json
// Request
{"fen": "..."}

// Response
{"fen": "...", "score_cp": 15}
```

### `POST /search`

```json
// Request - depth and time_limit_ms are optional
{"fen": "...", "depth": 12, "time_limit_ms": 5000}

// Response
{
  "fen": "...",
  "depth": 12,
  "score_cp": 30,       // null when a forced mate is found
  "score_mate": null,   // e.g. 3 = mate in 3, -2 = being mated in 2
  "best_move": "e2e4",
  "pv": ["e2e4", "e7e5", "g1f3"],
  "nodes": 1234567,
  "elapsed_ms": 842,
  "nps": 1465043
}
```

Concurrent searches are capped at `BITCRUSHER_MAX_CONCURRENT_SEARCHES` to prevent CPU oversubscription.

---

## Testing

Tests use a **mock `chess_engine` module** - no C++ build required.

```bash
cd web

# Install test deps
pip install pytest pytest-asyncio httpx

# Run all tests
python -m pytest tests/ -v

# Run a specific file or test
python -m pytest tests/test_models.py -v
python -m pytest tests/test_endpoints.py::TestSearch::test_basic_search -v
```

### What's covered (43 tests)

| File                      | Tests                                                                                                                                                                          |
| ------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `tests/test_models.py`    | FEN validation (empty, wrong field count, bad rank chars, wrong square count, invalid side/castling/en-passant, missing kings, bad clocks) + `SearchRequest` field constraints |
| `tests/test_endpoints.py` | Happy paths for all 4 endpoints, missing/invalid request fields, depth and time-limit boundary checks, engine error propagation (ValueError → 422, unhandled → 500)            |

---

## Load testing

> [!IMPORTANT]
> Make sure the API is running first (e.g., via `docker compose up`) before starting the load tests!


```bash
cd web/load_tests
uv run locust                          # web UI → http://localhost:8089
uv run locust --headless -u 20 -r 5   # 20 users, ramp 5/s
```
