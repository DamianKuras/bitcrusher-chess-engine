@echo off
call "%~dp0_setup_env.bat" || exit /b 1
cd /d "%~dp0.."
if not exist "build\benchmarks\CMakeCache.txt" cmake --preset benchmarks
if %errorlevel% neq 0 exit /b 1
cmake --build --preset benchmarks-release
if %errorlevel% neq 0 exit /b 1
if not exist benchmarks\results mkdir benchmarks\results
cd /d "%~dp0..\benchmarks"
..\bin\Release\BenchmarkRunner.exe --benchmark_out_format=json --benchmark_out=results\result.json --benchmark_report_aggregates_only=true --benchmark_min_warmup_time=0.2 %*
