"""pytest configuration - inject the mock chess_engine before any app import."""

import sys
import os

# Ensure the web/ directory is on the path so imports resolve correctly.
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

# Stub out the C++ extension before anything imports it.
from tests import mock_chess_engine  # noqa: E402

sys.modules["chess_engine"] = mock_chess_engine  # type: ignore[assignment]

import pytest
from fastapi.testclient import TestClient

# Mock required environment variables before Pydantic reads them in config.py
os.environ.setdefault("BITCRUSHER_CORS_ORIGINS", '["*"]')

from main import app


@pytest.fixture(scope="session")
def client() -> TestClient:
    with TestClient(app) as c:
        yield c
