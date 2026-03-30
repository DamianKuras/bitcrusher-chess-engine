"""Engine interaction layer.

Wraps the chess_engine pybind11 module with:
- asyncio.to_thread  - keeps the event loop free during C++ calls
- functools.lru_cache - avoids recomputing pure functions for repeated FENs
- asyncio.Semaphore  - caps concurrent searches at cpu_count to prevent
                       CPU oversubscription (set via init_engine())
"""

import asyncio
import logging
import os
import time
from functools import lru_cache

from fastapi import HTTPException

import chess_engine
from config import settings

log = logging.getLogger(__name__)

# Initialised in app lifespan - limits simultaneous alpha-beta searches.
_search_semaphore: asyncio.Semaphore | None = None


def init_engine() -> None:
    """Call once at application startup."""
    global _search_semaphore
    if settings.max_concurrent_searches:
        concurrency = settings.max_concurrent_searches
    else:
        try:
            concurrency = len(os.sched_getaffinity(0))
        except AttributeError:
            concurrency = os.cpu_count() or 1
    _search_semaphore = asyncio.Semaphore(concurrency)
    log.info("engine.init", extra={"concurrency": concurrency})


def _get_semaphore() -> asyncio.Semaphore:
    if _search_semaphore is None:
        raise RuntimeError("Engine not initialised - init_engine() was not called at startup")
    return _search_semaphore


# ---------------------------------------------------------------------------
# Cached pure-function wrappers (same FEN → same result, always)
# ---------------------------------------------------------------------------

@lru_cache(maxsize=settings.legal_moves_cache_size)
def _legal_moves_cached(fen: str) -> tuple[str, ...]:
    return tuple(chess_engine.legal_moves(fen))


@lru_cache(maxsize=settings.evaluate_cache_size)
def _evaluate_cached(fen: str) -> int:
    return chess_engine.evaluate(fen)


# ---------------------------------------------------------------------------
# Public async API
# ---------------------------------------------------------------------------

async def get_legal_moves(fen: str) -> tuple[str, ...]:
    try:
        return await asyncio.to_thread(_legal_moves_cached, fen)
    except ValueError as exc:
        raise HTTPException(status_code=422, detail=str(exc)) from exc
    except Exception as exc:
        log.exception("engine.legal_moves.error", extra={"fen": fen})
        raise HTTPException(status_code=500, detail="Engine error computing legal moves") from exc


async def get_evaluate(fen: str) -> int:
    try:
        return await asyncio.to_thread(_evaluate_cached, fen)
    except ValueError as exc:
        raise HTTPException(status_code=422, detail=str(exc)) from exc
    except Exception as exc:
        log.exception("engine.evaluate.error", extra={"fen": fen})
        raise HTTPException(status_code=500, detail="Engine error computing evaluation") from exc


async def get_search(fen: str, depth: int, time_limit_ms: int) -> dict:
    semaphore = _get_semaphore()
    try:
        async with semaphore:
            t0 = time.perf_counter()
            result = await asyncio.to_thread(chess_engine.search, fen, depth, time_limit_ms)
            elapsed = time.perf_counter() - t0
        result["elapsed_ms"] = int(elapsed * 1000)
        result["nps"] = int(result["nodes"] / elapsed) if elapsed > 0 else 0
        log.info(
            "engine.search.done",
            extra={
                "depth": depth,
                "elapsed_ms": result["elapsed_ms"],
                "nodes": result["nodes"],
                "nps": result["nps"],
            },
        )
        return result
    except ValueError as exc:
        raise HTTPException(status_code=422, detail=str(exc)) from exc
    except Exception as exc:
        log.exception("engine.search.error", extra={"fen": fen, "depth": depth})
        raise HTTPException(status_code=500, detail="Engine error during search") from exc
