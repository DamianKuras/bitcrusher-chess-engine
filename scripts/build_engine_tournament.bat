@echo off
REM build_engine_tournament.bat [WORK_DIR]
REM Configures (first time only) and builds the UCI engine for tournament use.
REM Prints the absolute path to Uci.exe on stdout; all errors go to stderr.
setlocal enabledelayedexpansion
call "%~dp0_setup_env.bat" || exit /b 1
if not "%~1"=="" (
    set WORK_DIR=%~f1
) else (
    for %%i in ("%~dp0..") do set WORK_DIR=%%~fi
)

set BUILD_DIR=!WORK_DIR!\build\tournament
set EXE=!WORK_DIR!\bin\Release\Uci.exe

if not exist "!BUILD_DIR!\CMakeCache.txt" (
    cmake -S "!WORK_DIR!" -B "!BUILD_DIR!" -G "Ninja Multi-Config" ^
        -DBITCRUSHER_BUILD_UCI=ON -DBITCRUSHER_WITH_BMI2=ON >nul 2>&1
    if !errorlevel! neq 0 ( echo Error: cmake configure failed. >&2 & exit /b 1 )
)

cmake --build "!BUILD_DIR!" --config Release --target Uci >nul 2>&1
if !errorlevel! neq 0 ( echo Error: cmake build failed. >&2 & exit /b 1 )

if not exist "!EXE!" ( echo Error: Binary not found after build. >&2 & exit /b 1 )
echo !EXE!
exit /b 0
