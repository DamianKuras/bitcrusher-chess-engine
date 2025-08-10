local google_benchmark_dir = path.join(DEPS_DIR,"benchmark-1.9.1")
ensure_dependency(
    "google-benchmark",
    "https://github.com/google/benchmark/archive/refs/tags/v1.9.1.zip",
    google_benchmark_dir
)

project "BenchmarkRunner"
    kind "ConsoleApp"
    defines { "BENCHMARK_STATIC_DEFINE" }
    defines { "BENCHMARK_ENABLE_LIBPFM" }
    dependson{
        "Engine", 
        "GoogleBenchmark"
    }
    files {
        path.join(BENCHMARKS_DIR,"**.cpp"),
        path.join(BENCHMARKS_DIR,"include","**.h")
    }
    links {
        "Engine",
        "GoogleBenchmark"
    }
    includedirs {
        path.join(google_benchmark_dir,"include"),
        ENGINE_INCLUDE_DIR,
        UCI_DIR
    }

-- Dependency GoogleBenchmark project as a static library
project "GoogleBenchmark"
    kind "StaticLib"
    defines { "BENCHMARK_STATIC_DEFINE" }
    files{
        path.join(google_benchmark_dir,"src","*.cc"),
        path.join(google_benchmark_dir,"include","benchmark","*.h")
    }
    includedirs{
        path.join(google_benchmark_dir,"include")
    }
    filter "system:linux"
        links{"pthread"}
    filter "system:windows"
        links { "Shlwapi" }



