# Bitcrusher Chess Engine

## Overview

Bitcrusher Chess Engine is a high-performance chess engine written in C++ that leverages bitboards for performance and efficient move generation.

## Features

- **Bitboard Representation:** Utilizes 64-bit integers to represent board states, ensuring fast bit-level operations.
- **Efficient Move Generation:** Implements bit manipulation techniques to generate legal moves quickly.

- **UCI Compatibility:** Easily integrates with popular chess GUIs using the Universal Chess Interface (UCI) protocol.
- **Cross-Platform:** Designed to compile and run on major operating systems including Windows, macOS, and Linux.

## Prerequisites

- **Build System:** [premake5](https://premake.github.io/).
- **Compiler:** A premake5 compliant compiler depending on the operating system.

## Installation

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/DamianKuras/bitcrusher-chess-engine.git
   ```

## Building and compiling on Windows

Generate a Visual Studio solution with all modules (engine, UCI, tests, benchmarks):

```bash
premake5 vs2022 --with-tests --with-uci --with-benchmarks
```

In folder /build there should be a file BitcrusherChessEngine.sln after running these command.
Open BitcrusherChessEngine.sln in Visual Studio  build, and run the desired projects.

## Building and compiling on Linux

Generate GNU Makefiles:

```bash
premake5 gmake2 --with-uci --with-tests --with-benchmarks
```

### Running and Testing

We provide cross-platform scripts (in `.sh` and `.bat` formats) in the `scripts/` directory to automate common tasks such as building, running tests, profiling, and launching the engine.

For comprehensive documentation on available scripts, see [Scripts Documentation](scripts/README.md).

**Examples:**

```bash
# Run the UCI engine (Release mode)
cd scripts
./run_uci.sh       # Linux/macOS
run_uci.bat        # Windows

# Run the test suite
./run_tests.sh     # Linux/macOS
run_tests.bat      # Windows

# Run benchmarks
./run_benchmarks.sh # Linux/macOS
run_benchmarks.bat  # Windows
```

## Documentation

The project uses [Doxygen](https://doxygen.nl/) to generate HTML and LaTeX documentation from the source code. To easily generate the documentation locally (which will safely clean up old folders first), use the provided script:

```bash
cd scripts
./generate_docs.sh      # Linux/macOS
generate_docs.bat       # Windows
```

## Acknowledgments

- Inspired by various open source chess engines and [chess programming wiki](https://www.chessprogramming.org/Main_Page).

## License

This project is licensed under the MIT License. See the LICENSE file for details.
