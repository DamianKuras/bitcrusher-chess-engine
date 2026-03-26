cmake_minimum_required(VERSION 3.25)
project(BitcrusherChessEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Mirror the old premake5 bin/<Config>/ output layout
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_SOURCE_DIR}/bin/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_SOURCE_DIR}/bin/Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin/RelWithDebInfo")

option(BITCRUSHER_BUILD_UCI        "Build UCI console application" OFF)
option(BITCRUSHER_BUILD_TESTS      "Build test suite"              OFF)
option(BITCRUSHER_BUILD_BENCHMARKS "Build benchmarks"              OFF)
option(BITCRUSHER_WITH_BMI2        "Enable BMI2/AVX2 instructions" OFF)

add_subdirectory(src/engine)

if(BITCRUSHER_BUILD_UCI)
    add_subdirectory(src/uci)
endif()
if(BITCRUSHER_BUILD_TESTS)
    add_subdirectory(tests)
endif()
if(BITCRUSHER_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
