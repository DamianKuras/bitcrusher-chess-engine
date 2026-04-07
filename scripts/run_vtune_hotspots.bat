@echo off
call "%~dp0_setup_env.bat" || exit /b 1
cd /d "%~dp0.."

echo Building benchmarks with debug symbols...
cmake --preset benchmarks -DCMAKE_CXX_FLAGS="/Zi" -DCMAKE_EXE_LINKER_FLAGS="/DEBUG"
if %errorlevel% neq 0 exit /b 1
cmake --build --preset benchmarks-release
if %errorlevel% neq 0 exit /b 1

if exist vtune_hotspots rmdir /s /q vtune_hotspots

echo Running VTune hotspots collection...
cd /d "%~dp0..\benchmarks"
vtune -collect hotspots -result-dir ..\vtune_hotspots -- ^
    ..\bin\Release\BenchmarkRunner.exe ^
    --benchmark_filter=%1 ^
    --benchmark_repetitions=20 ^
    --benchmark_min_warmup_time=0.5
if %errorlevel% neq 0 ( cd /d "%~dp0.." & exit /b 1 )
cd /d "%~dp0.."

echo.
echo Hotspot report:
vtune -report hotspots -r vtune_hotspots -format=text -report-knob show-issues=false

echo.
echo NOTE: rebuild without symbols before running regular benchmarks:
echo   scripts\run_benchmarks.bat
