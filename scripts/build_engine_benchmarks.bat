@echo off
REM build_engine_benchmarks.bat [WORK_DIR]
REM Configures (first time only) and builds the BenchmarkRunner.
REM Prints the absolute path to BenchmarkRunner.exe on stdout; all errors go to stderr.
setlocal enabledelayedexpansion
call "%~dp0_setup_env.bat" || exit /b 1
if not "%~1"=="" (
    set WORK_DIR=%~f1
) else (
    for %%i in ("%~dp0..") do set WORK_DIR=%%~fi
)

set BUILD_DIR=!WORK_DIR!\build\benchmarks
set EXE=!WORK_DIR!\bin\Release\BenchmarkRunner.exe

if not exist "!BUILD_DIR!\CMakeCache.txt" (
    cmake -S "!WORK_DIR!" -B "!BUILD_DIR!" -G "Ninja Multi-Config" ^
        -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
        -DBITCRUSHER_BUILD_BENCHMARKS=ON -DBITCRUSHER_WITH_BMI2=ON >nul 2>&1
    if !errorlevel! neq 0 ( echo Error: cmake configure failed. >&2 & exit /b 1 )
)

cmake --build "!BUILD_DIR!" --config Release --target BenchmarkRunner >nul 2>&1
if !errorlevel! neq 0 ( echo Error: cmake build failed. >&2 & exit /b 1 )

if not exist "!EXE!" ( echo Error: Binary not found after build. >&2 & exit /b 1 )
echo !EXE!
exit /b 0
