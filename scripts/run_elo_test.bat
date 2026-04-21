@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0.."

where fastchess >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: 'fastchess' could not be found.
    echo Please ensure it is installed and added to your system PATH.
    exit /b 1
)

set TC_TYPE=%~1
if "%TC_TYPE%"=="" set TC_TYPE=2phase
set BRANCH_DEV=%~2
if "%BRANCH_DEV%"=="" set BRANCH_DEV=dev

if /i "%TC_TYPE%"=="fstc" (
    set TC=3+0.03
    set SPRT_ELO1=10
) else if /i "%TC_TYPE%"=="stc" (
    set TC=10+0.1
    set SPRT_ELO1=5
) else if /i "%TC_TYPE%"=="ltc" (
    set TC=60+0.6
    set SPRT_ELO1=5
) else if /i "%TC_TYPE%"=="2phase" (
    rem handled separately below
) else (
    echo Error: Unknown time control type '%TC_TYPE%'. Use fstc^|stc^|ltc^|2phase.
    exit /b 1
)

set BRANCH_BASE=main
set REPO_ROOT=%CD%

echo  Starting Automated %TC_TYPE% SPRT Elo Testing
echo  Comparing: %BRANCH_DEV% (New) vs %BRANCH_BASE% (Base)
if /i "%TC_TYPE%"=="2phase" (
    echo  Phase 1: 3+0.03, elo1=10, max 200 rounds  -^>  Phase 2: 10+0.1, elo1=5
) else (
    echo  Time Control: %TC% ^| SPRT elo0=0, elo1=%SPRT_ELO1%
)
echo  Concurrency : 4 threads
echo ================================================================

set TMP_DIR=%TEMP%\bitcrusher_sprt_%RANDOM%
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

echo Cloning repositories...
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

echo Compiling Base Engine (%BRANCH_BASE%)...
call "%~dp0build_engine_tournament.bat" "%BASE_DIR%" >nul
if %errorlevel% neq 0 ( echo Error: Failed to compile %BRANCH_BASE%. & goto cleanup_fail )

echo Compiling Dev Engine (%BRANCH_DEV%)...
call "%~dp0build_engine_tournament.bat" "%DEV_DIR%" >nul
if %errorlevel% neq 0 ( echo Error: Failed to compile %BRANCH_DEV%. & goto cleanup_fail )

echo Compilations successful.
cd /d "%TMP_DIR%"

if /i "%TC_TYPE%"=="2phase" (
    echo.
    echo === Phase 1: Fast Filter (3+0.03, elo1=10, max 200 rounds) ===
    set phase1_log=!TMP_DIR!\phase1.log
    fastchess -engine cmd="!DEV_DIR!\bin\Release\Uci.exe" name="Bitcrusher_!BRANCH_DEV!" -engine cmd="!BASE_DIR!\bin\Release\Uci.exe" name="Bitcrusher_!BRANCH_BASE!" -each tc=3+0.03 -rounds 200 -games 2 -repeat -concurrency 4 -openings file="!REPO_ROOT!\data\pgn\8move_v3.pgn" format=pgn order=random -sprt elo0=0.0 elo1=10 alpha=0.05 beta=0.05 -recover 2>&1 | powershell -Command "& { $input | Tee-Object -FilePath '!phase1_log!' }"
    echo.
    findstr /i "h1" "!phase1_log!" | findstr /i "accept" >nul 2>&1
    if !errorlevel! == 0 (
        echo Phase 1: H1 accepted. Proceeding to Phase 2.
        echo.
        echo === Phase 2: Confirmation (10+0.1, elo1=5, max 500 rounds) ===
        fastchess -engine cmd="!DEV_DIR!\bin\Release\Uci.exe" name="Bitcrusher_!BRANCH_DEV!" -engine cmd="!BASE_DIR!\bin\Release\Uci.exe" name="Bitcrusher_!BRANCH_BASE!" -each tc=10+0.1 -rounds 500 -games 2 -repeat -concurrency 4 -openings file="!REPO_ROOT!\data\pgn\8move_v3.pgn" format=pgn order=random -sprt elo0=0.0 elo1=5 alpha=0.05 beta=0.05 -recover
    ) else (
        findstr /i "h0" "!phase1_log!" | findstr /i "accept" >nul 2>&1
        if !errorlevel! == 0 (
            echo Phase 1: H0 accepted. Patch rejected.
        ) else (
            echo Phase 1: Inconclusive (round cap reached).
            echo The improvement is likely small (0-10 Elo). Run 'stc' directly for fine-grained testing.
        )
    )
) else (
    fastchess -engine cmd="!DEV_DIR!\bin\Release\Uci.exe" name="Bitcrusher_!BRANCH_DEV!" -engine cmd="!BASE_DIR!\bin\Release\Uci.exe" name="Bitcrusher_!BRANCH_BASE!" -each tc=!TC! -rounds 500 -games 2 -repeat -concurrency 4 -openings file="!REPO_ROOT!\data\pgn\8move_v3.pgn" format=pgn order=random -sprt elo0=0.0 elo1=!SPRT_ELO1! alpha=0.05 beta=0.05 -recover
)

echo.
echo %TC_TYPE% match completed.
cd /d "%REPO_ROOT%"
rmdir /s /q "%TMP_DIR%"
echo Done.
exit /b 0

:cleanup_fail
cd /d "%REPO_ROOT%"
if exist "%TMP_DIR%" rmdir /s /q "%TMP_DIR%"
exit /b 1
