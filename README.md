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
Open bitcrusher.sln in Visual Studio, build, and run the desired projects.
## Building and compiling on Linux

Generate GNU Makefiles:

```bash
premake5 gmake2 --with-uci --with-tests --with-benchmarks
```

### Running the Engine (UCI)

```bash
cd build && make clean && make Uci config=debug_x64
```

### Running tests
```bash
# Run fast validation suite (exclude long tests)
cd build && make Tests config=debug_x64 && ../bin/Debug/Tests --gtest_filter=-*slow
# Run all tests
cd build && make Tests config=debug_x64 && ../bin/Debug/Tests 
```

### Running benchmarks
```bash
# Run fast benchmarks suite (exclude long benchmarks)
cd build && make clean && make BenchmarkRunner config=release_x64 && cd  && ./bin/Release/BenchmarkRunner --benchmark_filter=-*slow
# Run all benchmarks
cd build && make clean && make BenchmarkRunner config=release_x64 && cd  && ./bin/Release/BenchmarkRunner
# Run specific benchmark for example BM_perft/4/32 [name/depth/repetition for perft benchmark]
cd build && make clean && make BenchmarkRunner config=release_x64 && cd  && ./bin/Release/BenchmarkRunner --benchmark_filter=BM_perft/4/32
```

## Acknowledgments
- Inspired by various open source chess engines and [chess programming wiki](https://www.chessprogramming.org/Main_Page).


## License
This project is licensed under the MIT License. See the LICENSE file for details.