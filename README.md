# Bitcrusher Chess Engine

A UCI-compatible chess engine written in C++23 using bitboard representation for fast move generation and search.

**Estimated strength: ~1921 Elo**

## Features

- **Bitboard representation:** 12 x 64-bit piece bitboards with fast bit-level operations
- **Legal move generation:** per-piece generators with pinned-piece and check restriction contexts
- **Alpha-beta search:** quiescence search, MVV-LVA move ordering, transposition table, multi-threaded via `search_manager`
- **Tapered evaluation:** hand-crafted PST-based eval with separate middlegame/endgame weights
- **Web UI:** React frontend backed by FastAPI with in-process pybind11 engine bindings (no UCI subprocess)
- **UCI compatibility:** works with any UCI-compatible chess GUI

## Prerequisites

| Tool | Purpose |
|------|---------|
| CMake 3.25+ | Build system |
| Ninja | Generator (used by all presets) |
| vcpkg | Dependency manager (GoogleTest, Google Benchmark) |
| MSVC / GCC / Clang | C++23-capable compiler |

The UCI engine has no external dependencies. Only tests and benchmarks require vcpkg.

## Building

The project uses CMake presets. Scripts in `scripts/` wrap the common workflows and are the preferred way to build and run.

### Windows

```batch
scripts\run_uci.bat              # Build + launch UCI engine (Release)
scripts\run_tests.bat            # Build + run GoogleTest suite (skips slow perft)
scripts\run_all_tests.bat        # Build + run all tests including slow perft
scripts\run_tests_debug.bat      # Debug build
scripts\run_tests_no_bmi2.bat    # Without BMI2 intrinsics
scripts\run_benchmarks.bat       # Build + run Google Benchmark suite
```

### Linux / macOS

```bash
scripts/run_uci.sh
scripts/run_tests.sh
scripts/run_benchmarks.sh
```

### Manual CMake

If you prefer to invoke CMake directly, presets are defined in `CMakePresets.json`:

```bash
# UCI engine (no vcpkg required)
cmake --preset uci
cmake --build --preset uci-release

# Tests (requires VCPKG_ROOT to be set)
cmake --preset tests
cmake --build --preset tests-release
bin/Release/Tests.exe

# Benchmarks
cmake --preset benchmarks
cmake --build --preset benchmarks-release
```

Available presets: `uci`, `tests`, `tests-no-bmi2`, `benchmarks`, and Linux equivalents (`linux-uci-release`, `linux-tests-release`, etc.).

### BMI2 / AVX2

Pass `-DBITCRUSHER_WITH_BMI2=ON` (or use any preset that already sets it) to enable `_pext_u64`, `_pdep_u64`, `_tzcnt_u64`, `_lzcnt_u64`.

## Project Structure

```
src/
  engine/include/         # Header-only engine library
    legal_move_generators/  # Per-piece legal move generators
    evaluation.hpp          # Hand-crafted PST tapered eval
    search.hpp              # Alpha-beta + quiescence search
    search_manager.hpp      # Multi-threaded search, transposition table
    board_state.hpp         # 12 x 64-bit piece bitboards
    fen_formatter.hpp       # FEN parsing and serialisation
  uci/
    uci_handler.hpp         # UCI command parser
    uci.cpp                 # Entry point
tests/engine/             # GoogleTest suite
benchmarks/               # Google Benchmark suite
web/                      # React + FastAPI web app (see below)
scripts/                  # Build, test, lint, and tooling scripts
data/fens/                # FEN position files used by benchmarks
```

## Other Scripts

Run once after cloning to activate the pre-commit formatting hook:

```batch
scripts\install_hooks.bat   # Windows
scripts/install_hooks.sh    # Linux/macOS
```

```batch
scripts\format_code.bat              # clang-format
scripts\lint_code.bat                # clang-tidy
scripts\create_compile_commands.bat  # Generate compile_commands.json
scripts\clean_workspace.bat          # Remove bin/ obj/ build/
scripts\generate_docs.bat            # Generate Doxygen HTML/LaTeX docs
scripts\estimate_elo.bat             # Gauntlet vs Stockfish (auto-downloaded)
scripts\run_elo_test.bat             # SPRT Elo test vs reference branch
scripts\build_engine_tournament.bat  # Release + BMI2 binary, prints path to Uci.exe
```

## Web App

A React chess UI backed by FastAPI. The engine runs in-process via pybind11, no UCI subprocess.

```bash
docker compose -f web/docker-compose.yml up --build
```

| URL | What |
|-----|------|
| `http://localhost` | Chess frontend (play vs engine) |
| `http://localhost:8000/docs` | Swagger UI |

See [web/README.md](web/README.md) for configuration, API reference, testing, and load testing.

## Documentation

```batch
scripts\generate_docs.bat    # Windows
scripts/generate_docs.sh     # Linux/macOS
```

Generates Doxygen HTML + LaTeX from source into `docs/`.

## Acknowledgments

Inspired by various open-source chess engines and the [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page).

## License

MIT License. See the LICENSE file for details.
