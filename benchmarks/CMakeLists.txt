find_package(benchmark REQUIRED)

file(GLOB BENCHMARK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(BenchmarkRunner ${BENCHMARK_SOURCES})

target_compile_definitions(BenchmarkRunner PRIVATE BENCHMARK_STATIC_DEFINE)
target_link_libraries(BenchmarkRunner PRIVATE Engine benchmark::benchmark_main)
target_include_directories(BenchmarkRunner PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src/uci"
)

if(WIN32)
    target_link_libraries(BenchmarkRunner PRIVATE Shlwapi)
endif()
