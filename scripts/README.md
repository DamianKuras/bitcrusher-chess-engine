# Scripts Documentation

This directory contains utility scripts to automate common tasks such as building, testing, benchmarking, and interacting with the Bitcrusher Chess Engine.

All scripts are provided in both `.sh` (for Linux/macOS) and `.bat` (for Windows) formats to ensure cross-platform compatibility.

## Prerequisites

The build system uses **CMake** (3.25+) with **Ninja** and **MSVC** on Windows, and **GCC/Clang** on Linux.

- **Windows**: Visual Studio 2022+ must be installed. The `_setup_env.bat` helper (called automatically by every `.bat` script) locates the VS installation via `vswhere` and initialises the MSVC compiler environment. No manual setup required.
- **Linux**: Install `cmake`, `ninja-build`, and a C++23-capable compiler.
- **Tests / Benchmarks**: require `vcpkg` with `VCPKG_ROOT` set, or use the VS-bundled vcpkg (set automatically when using the VS Developer environment).

## General Usage

Run all scripts from the **repository root**:

### Windows
```cmd
scripts\run_tests.bat
```

### Linux / macOS / WSL
```bash
scripts/run_tests.sh
```

## Available Scripts

### Building and Running
- **`run_uci`**: Builds the engine in Release mode and starts the UCI interface.
- **`run_uci_debug`**: Builds the engine in Debug mode and starts the UCI interface.
- **`run_uci_commands`**: Builds the engine in Release mode and feeds `uci_commands.txt` via standard input to the engine.
- **`build_engine_tournament`** *(Windows)*: Builds a Release+BMI2 UCI binary optimised for tournament use. Prints the absolute path to `Uci.exe` on stdout. Used internally by the GA training system.

### Testing
- **`run_all_tests`**: Builds and runs the entire GoogleTest suite, including slow and long-running tests.
- **`run_tests`**: Builds and runs the GoogleTest suite in Release mode. Skips long tests by default.
- **`run_tests_debug`**: Builds and runs the test suite in Debug mode.
- **`run_tests_no_bmi2`**: Builds and runs the test suite in Release mode without BMI2 instruction set optimizations.

### Benchmarking
- **`run_benchmarks`**: Builds and runs the Google Benchmark suite. Results are saved in `benchmarks/results/result.json`.

  Any extra arguments are forwarded to the benchmark binary. Use `--benchmark_filter=<regex>` to run a subset:
  ```cmd
  scripts\run_benchmarks.bat --benchmark_filter=SearchThreading
  ```
  ```bash
  scripts/run_benchmarks.sh --benchmark_filter=SearchThreading
  ```
  Use `--benchmark_list_tests` to print all available benchmark names without running them.

### Code Quality & Maintenance
- **`format_code`**: Runs `clang-format` on all C++ source and header files in the repository.
- **`lint_code`**: Runs `clang-tidy` on all C++ source and header files (generates `compile_commands.json` if missing).
- **`clean_workspace`**: Completely removes the `bin/`, `obj/`, and `build/` directories.
- **`create_compile_commands`**: Generates a `compile_commands.json` file for integration with language servers (like clangd).

### Analysis & Debugging
- **`run_valgrind`**: Builds the engine in Release mode and runs it under Valgrind (memcheck/cachegrind). *Requires Linux or WSL.*

### UCI Compliance
- **`fastchess_vs_sf`**: Runs a quick match against Stockfish using fastchess. Requires `stockfish` and `fastchess` in PATH.

### Elo Estimation
- **`run_elo_test`** *(Windows)*: SPRT Elo test against a reference branch using fastchess. Requires `fastchess` in PATH and `data/pgn/8move_v3.pgn`.
- **`estimate_elo`**: Estimates the Elo rating of the engine by running a gauntlet against Stockfish. Requires Python 3 and `fastchess` in PATH. Downloads Stockfish automatically on first run.

---

> **Internal**: `_setup_env.bat` is called automatically by every `.bat` script — it sets up the MSVC compiler, Ninja, and CMake from the VS installation. It does not need to be invoked directly.
