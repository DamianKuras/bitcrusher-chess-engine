# ============================================================
# Stage 1 - Build the pybind11 C++ extension module
# Ubuntu 24.04 ships GCC 13 natively (required for C++23).
# ============================================================
FROM ubuntu:24.04 AS cpp-builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc g++ cmake ninja-build \
    python3.12 python3.12-dev python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir pybind11 --break-system-packages

WORKDIR /src

COPY CMakeLists.txt .
COPY src/engine ./src/engine
COPY web/bindings ./web/bindings

RUN cmake -S . -B /build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBITCRUSHER_BUILD_PYTHON_BINDINGS=ON \
    && cmake --build /build --target chess_engine -j"$(nproc)"

# ============================================================
# Stage 2 - Runtime 
# ============================================================
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    python3.12 python3.12-venv ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=ghcr.io/astral-sh/uv:latest /uv /usr/local/bin/uv

# Create a non-root user for running the service.
RUN groupadd --gid 1001 appgroup \
    && useradd --uid 1001 --gid appgroup --shell /bin/bash --create-home appuser

WORKDIR /app

RUN uv venv /opt/venv --python python3.12
ENV PATH="/opt/venv/bin:$PATH"

COPY --from=cpp-builder /src/web/chess_engine*.so /app/

COPY web/pyproject.toml /app/
RUN uv pip install --no-cache -r pyproject.toml

COPY web/main.py web/models.py web/engine.py web/config.py /app/

# Transfer ownership to the non-root user.
RUN chown -R appuser:appgroup /app /opt/venv

USER appuser

EXPOSE 8000

HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD python3 -c "import urllib.request; urllib.request.urlopen('http://localhost:8000/health')" || exit 1

CMD ["gunicorn", "main:app", "--workers", "2", "--worker-class", "uvicorn.workers.UvicornWorker", "--bind", "0.0.0.0:8000"]
