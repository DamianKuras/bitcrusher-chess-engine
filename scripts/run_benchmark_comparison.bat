@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0.."

set BRANCH_DEV=%~1
if "%BRANCH_DEV%"=="" set BRANCH_DEV=dev

set BRANCH_BASE=main
set REPO_ROOT=%CD%

echo  Starting Benchmark Comparison
echo  Comparing: %BRANCH_DEV% (New) vs %BRANCH_BASE% (Base)
echo ================================================================

set TMP_DIR=%TEMP%\bitcrusher_bench_%RANDOM%
set BASE_DIR=%TMP_DIR%\base
set DEV_DIR=%TMP_DIR%\dev

mkdir "%TMP_DIR%"

echo Detecting remote repository URL...
for /f "usebackq tokens=*" %%i in (`git config --get remote.origin.url`) do (
    set REMOTE_URL=%%i
)
if "!REMOTE_URL!"=="" (
    echo Error: Could not determine remote origin URL.
    goto cleanup_fail
)
echo Using remote: !REMOTE_URL!

echo Cloning base branch '%BRANCH_BASE%'...
git clone -q --branch "%BRANCH_BASE%" "!REMOTE_URL!" "%BASE_DIR%"
if %errorlevel% neq 0 ( echo Error: Failed to clone base. & goto cleanup_fail )

git ls-remote --exit-code --heads "!REMOTE_URL!" "%BRANCH_DEV%" >nul 2>nul
if !errorlevel! == 0 (
    echo Cloning dev branch '%BRANCH_DEV%' from remote...
    git clone -q --branch "%BRANCH_DEV%" "!REMOTE_URL!" "%DEV_DIR%"
    if !errorlevel! neq 0 ( echo Error: Failed to clone dev. & goto cleanup_fail )
) else (
    echo Dev branch '%BRANCH_DEV%' not found on remote, copying current local state...
    robocopy "%REPO_ROOT%" "%DEV_DIR%" /E /XD bin obj build /NP /NFL /NDL >nul
    if !errorlevel! geq 8 ( echo Error: Failed to copy local state. & goto cleanup_fail )
)

echo Compiling base benchmarks (%BRANCH_BASE%)...
call "%~dp0build_engine_benchmarks.bat" "%BASE_DIR%" >nul
if %errorlevel% neq 0 ( echo Error: Failed to compile base. & goto cleanup_fail )

echo Compiling dev benchmarks (%BRANCH_DEV%)...
call "%~dp0build_engine_benchmarks.bat" "%DEV_DIR%" >nul
if %errorlevel% neq 0 ( echo Error: Failed to compile dev. & goto cleanup_fail )

echo Compilations successful. Running benchmarks...

set BASE_JSON=%TMP_DIR%\base_bench.json
set DEV_JSON=%TMP_DIR%\dev_bench.json

REM Run from the benchmarks/ subdirectory so relative data paths (../data/fens/) resolve.
echo Running base benchmarks...
pushd "%BASE_DIR%\benchmarks"
"%BASE_DIR%\bin\Release\BenchmarkRunner.exe" ^
    --benchmark_out_format=json --benchmark_out="!BASE_JSON!" ^
    --benchmark_repetitions=3 ^
    --benchmark_report_aggregates_only=true ^
    --benchmark_min_warmup_time=0.2 2>nul
if %errorlevel% neq 0 ( popd & echo Error: Base benchmark run failed. & goto cleanup_fail )
popd

echo Running dev benchmarks...
pushd "%DEV_DIR%\benchmarks"
"%DEV_DIR%\bin\Release\BenchmarkRunner.exe" ^
    --benchmark_out_format=json --benchmark_out="!DEV_JSON!" ^
    --benchmark_repetitions=3 ^
    --benchmark_report_aggregates_only=true ^
    --benchmark_min_warmup_time=0.2 2>nul
if %errorlevel% neq 0 ( popd & echo Error: Dev benchmark run failed. & goto cleanup_fail )
popd

echo.
python "%~dp0compare_benchmarks.py" "!BASE_JSON!" "!DEV_JSON!" "%BRANCH_BASE%" "%BRANCH_DEV%"
echo.

cd /d "%REPO_ROOT%"
rmdir /s /q "%TMP_DIR%"
echo Done.
exit /b 0

:cleanup_fail
cd /d "%REPO_ROOT%"
if exist "%TMP_DIR%" rmdir /s /q "%TMP_DIR%"
exit /b 1
