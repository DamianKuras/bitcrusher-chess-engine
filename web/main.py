"""Bitcrusher Chess Engine - FastAPI backend."""

import logging
import logging.config
import os
import time
from contextlib import asynccontextmanager

from fastapi import FastAPI, Request, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from fastapi.staticfiles import StaticFiles
from slowapi import Limiter, _rate_limit_exceeded_handler
from slowapi.errors import RateLimitExceeded
from slowapi.util import get_remote_address

import engine
from config import settings
from models import (
    EvaluateResponse,
    FENRequest,
    LegalMovesResponse,
    SearchRequest,
    SearchResponse,
)

# ---------------------------------------------------------------------------
# Logging - JSON-style structured output suitable for log aggregators.
# ---------------------------------------------------------------------------

logging.config.dictConfig(
    {
        "version": 1,
        "disable_existing_loggers": False,
        "formatters": {
            "json": {
                "format": '{"time":"%(asctime)s","level":"%(levelname)s","logger":"%(name)s","msg":%(message)s}',
                "datefmt": "%Y-%m-%dT%H:%M:%S",
            }
        },
        "handlers": {
            "console": {
                "class": "logging.StreamHandler",
                "formatter": "json",
            }
        },
        "root": {"level": "INFO", "handlers": ["console"]},
    }
)

log = logging.getLogger(__name__)

# ---------------------------------------------------------------------------
# Rate limiter - disabled when rate_limit_per_minute == 0.
# ---------------------------------------------------------------------------

_rate_limit = (
    f"{settings.rate_limit_per_minute}/minute"
    if settings.rate_limit_per_minute > 0
    else "10000/minute"  # effectively unlimited
)
limiter = Limiter(key_func=get_remote_address, default_limits=[_rate_limit])


# ---------------------------------------------------------------------------
# App lifecycle
# ---------------------------------------------------------------------------


@asynccontextmanager
async def lifespan(app: FastAPI):
    log.info('"application startup"')
    engine.init_engine()
    yield
    log.info('"application shutdown"')


app = FastAPI(
    title="Bitcrusher Chess API",
    description="REST API powered by the Bitcrusher chess engine via pybind11 bindings.",
    version="0.1.0",
    lifespan=lifespan,
)
app.state.limiter = limiter
app.add_exception_handler(RateLimitExceeded, _rate_limit_exceeded_handler)

# ---------------------------------------------------------------------------
# Middleware
# ---------------------------------------------------------------------------

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_origins,
    allow_methods=["GET", "POST"],
    allow_headers=["Content-Type"],
)


@app.middleware("http")
async def log_requests(request: Request, call_next):
    t0 = time.perf_counter()
    response = await call_next(request)
    elapsed_ms = int((time.perf_counter() - t0) * 1000)
    log.info(
        '"%s %s %s"',
        request.method,
        request.url.path,
        response.status_code,
        extra={"elapsed_ms": elapsed_ms},
    )
    return response


# ---------------------------------------------------------------------------
# Global exception handler - prevents raw tracebacks reaching the client.
# ---------------------------------------------------------------------------

@app.exception_handler(Exception)
async def unhandled_exception_handler(request: Request, exc: Exception):
    log.exception('"unhandled exception"', extra={"path": request.url.path})
    return JSONResponse(
        status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
        content={"detail": "Internal server error"},
    )


# ---------------------------------------------------------------------------
# Routes
# ---------------------------------------------------------------------------

@app.get("/health", tags=["meta"])
async def health():
    """Returns 200 when the engine is initialised and ready."""
    if engine._search_semaphore is None:
        return JSONResponse(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            content={"status": "unavailable", "reason": "engine not initialised"},
        )
    return {"status": "ok"}


@app.post("/legal-moves", response_model=LegalMovesResponse, tags=["position"])
@limiter.limit(_rate_limit)
async def legal_moves(request: Request, req: FENRequest):
    """Return all legal moves for the given position."""
    moves = await engine.get_legal_moves(req.fen)
    return LegalMovesResponse(fen=req.fen, moves=moves, count=len(moves))


@app.post("/evaluate", response_model=EvaluateResponse, tags=["position"])
@limiter.limit(_rate_limit)
async def evaluate(request: Request, req: FENRequest):
    """Static evaluation - fast, no search."""
    score = await engine.get_evaluate(req.fen)
    return EvaluateResponse(fen=req.fen, score_cp=score)


@app.post("/search", response_model=SearchResponse, response_model_exclude_none=True, tags=["search"])
@limiter.limit(_rate_limit)
async def search(request: Request, req: SearchRequest):
    """Alpha-beta search to the given depth."""
    result = await engine.get_search(req.fen, req.depth, req.time_limit_ms)
    return SearchResponse(
        fen=req.fen,
        depth=req.depth,
        score_cp=result["score_cp"],
        score_mate=result["score_mate"],
        best_move=result["best_move"],
        pv=result["pv"],
        nodes=result["nodes"],
        elapsed_ms=result.get("elapsed_ms", 0),
        nps=result.get("nps", 0),
    )


# Serve the React build in production (Docker). Must be mounted last so API
# routes take precedence. html=True makes all unknown paths serve index.html
# which is required for client-side routing.
_static = os.path.join(os.path.dirname(__file__), "static")
if os.path.isdir(_static):
    app.mount("/", StaticFiles(directory=_static, html=True), name="frontend")
