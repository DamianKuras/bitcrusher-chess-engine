# Scripts Documentation

This directory contains utility scripts to automate common tasks such as building, testing, benchmarking, and interacting with the Bitcrusher Chess Engine. 

All scripts are provided in both `.sh` (for Linux/macOS) and `.bat` (for Windows) formats to ensure cross-platform compatibility.

## General Usage

All scripts should be executed from within the `scripts/` directory or from the repository root. The scripts will automatically navigate to the repository root before executing commands.

### Windows
```cmd
cd scripts
run_tests.bat
```

### Linux / macOS / WSL
```bash
cd scripts
./run_tests.sh
```

## Available Scripts

### Building and Running
- **`run_uci`**: Builds the engine in Release mode and starts the UCI interface.
- **`run_uci_debug`**: Builds the engine in Debug mode and starts the UCI interface.
- **`run_uci_commands`**: Builds the engine in Release mode and feeds `uci_commands.txt` via standard input to the engine.

### Testing
- **`run_all_tests`**: Builds and runs the entire GoogleTest suite, including slow and long-running tests.
- **`run_tests`**: Builds and runs the GoogleTest suite in Release mode. Skips long tests by default.
- **`run_tests_debug`**: Builds and runs the test suite in Debug mode.
- **`run_tests_no_bmi2`**: Builds and runs the test suite in Release mode without BMI2 instruction set optimizations.

### Benchmarking
- **`run_benchmarks`**: Builds and runs the Google Benchmark suite. Results are saved in `benchmarks/results/result.json`.

### Code Quality & Maintenance
- **`format_code`**: Runs `clang-format` on all C++ source and header files in the repository.
- **`lint_code`**: Runs `clang-tidy` on all C++ source and header files (generates `compile_commands.json` if missing).
- **`clean_workspace`**: Completely removes the `bin/`, `obj/`, and `build/` directories.
- **`generate_docs`**: Runs Doxygen to generate HTML/LaTeX documentation in the repository root.

### Analysis & Debugging
- **`run_valgrind`**: Builds the engine in Release mode and runs it under Valgrind (memcheck/cachegrind). *Requires Linux or WSL.*
- **`create_compile_commands`**: Generates a `compile_commands.json` file for integration with language servers (like clangd).

### UCI Compliance
- **`fastchess_compliance`**: Uses [fastchess](https://github.com/Disservin/fastchess) to run a UCI compliance check on the engine. Note: ensure `fastchess` is available in your system path.

### Testing vs dev branch
- **`run_elo_test_stc`**: Fast Short Time Control (STC) Elo testing script (10+0.1). Automatically clones the repository into isolated folders, compiles the engines, and runs a highly concurrent `fastchess` SPRT match. *Requires `data/pgn/8move_v3.pgn`.*
- **`run_elo_test_ltc`**: Long Time Control (LTC) Elo testing script (60+0.6) for tournament-grade rating verification. *Requires `data/pgn/8move_v3.pgn`.*

### Testing vs Stockfish
- **`fastchess_vs_sf`**: Uses [cutechess-cli](http://cutechess.sourceforge.net/) to run a quick 1-round match against Stockfish. Note: ensure `stockfish` and `cutechess-cli` are available in your system path.


### Elo Estimation
- **`estimate_elo`**: Estimates the Elo rating of the engine using python script. Will download Stockfish and run a gauntlet against it to estimate the Elo rating of the engine. Requires Python 3 and fastchess in PATH.