@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0.."

where fastchess >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: 'fastchess' could not be found.
    echo Please ensure it is installed and added to your system PATH.
    exit /b 1
)

set BRANCH_DEV=%~1
if "%BRANCH_DEV%"=="" set BRANCH_DEV=dev
set BRANCH_BASE=main
set REPO_ROOT=%CD%

echo  Starting Automated LTC SPRT Elo Testing
echo  Comparing: %BRANCH_DEV% (New) vs %BRANCH_BASE% (Base)
echo  Time Control: 60+0.6
echo  Concurrency : 4 threads
echo ================================================================

REM Create isolated temp directories
set TMP_DIR=%TEMP%\bitcrusher_sprt_%RANDOM%
set BASE_DIR=%TMP_DIR%\base
set DEV_DIR=%TMP_DIR%\dev

mkdir "%TMP_DIR%"

echo Detecting remote repository URL...
for /f "usebackq tokens=*" %%i in (`git config --get remote.origin.url`) do (
    set REMOTE_URL=%%i
)
if "%REMOTE_URL%"=="" (
    echo Error: Could not determine remote origin URL.
    exit /b 1
)
echo Using remote: !REMOTE_URL!

echo Cloning clean repositories from remote to temporary directories...
git clone -q "!REMOTE_URL!" "%BASE_DIR%"
if %errorlevel% neq 0 ( echo Failed to clone base. & exit /b 1 )
git clone -q "!REMOTE_URL!" "%DEV_DIR%"
if %errorlevel% neq 0 ( echo Failed to clone dev. & exit /b 1 )

echo Checking out branches...
cd /d "%BASE_DIR%"
git fetch --all -q
git checkout -q "%BRANCH_BASE%"
if %errorlevel% neq 0 ( echo Failed to checkout %BRANCH_BASE%. & exit /b 1 )

cd /d "%DEV_DIR%"
git fetch --all -q
git checkout -q "%BRANCH_DEV%"
if %errorlevel% neq 0 ( echo Failed to checkout %BRANCH_DEV%. & exit /b 1 )

REM Find MSBuild
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBUILD="%%i"
)
if not defined MSBUILD (
    echo Error: Could not find MSBuild.exe.
    exit /b 1
)

echo Compiling Base Engine (%BRANCH_BASE%)...
cd /d "%BASE_DIR%"
premake5 vs2022 --with-uci --with-bmi2 >nul
cd build
%MSBUILD% BitcrusherChessEngine.sln /p:Configuration=Release /p:Platform=x64 /m >nul
if not exist "%BASE_DIR%\bin\Release\Uci.exe" (
    echo Error: Failed to compile %BRANCH_BASE%.
    exit /b 1
)

echo Compiling Dev Engine (%BRANCH_DEV%)...
cd /d "%DEV_DIR%"
premake5 vs2022 --with-uci --with-bmi2 >nul
cd build
%MSBUILD% BitcrusherChessEngine.sln /p:Configuration=Release /p:Platform=x64 /m >nul
if not exist "%DEV_DIR%\bin\Release\Uci.exe" (
    echo Error: Failed to compile %BRANCH_DEV%.
    exit /b 1
)

echo Compilations successful. Starting LTC fastchess match...
cd /d "%TMP_DIR%"

fastchess -engine cmd="%DEV_DIR%\bin\Release\Uci.exe" name="Bitcrusher_%BRANCH_DEV%" -engine cmd="%BASE_DIR%\bin\Release\Uci.exe" name="Bitcrusher_%BRANCH_BASE%" -each tc=60+0.6 -rounds 500 -games 2 -repeat -concurrency 4 -openings file="%REPO_ROOT%\data\pgn\8move_v3.pgn" format=pgn order=random -sprt elo0=0.0 elo1=5.0 alpha=0.05 beta=0.05

echo.
echo LTC Match completed. Cleaning up temporary files...
cd /d "%REPO_ROOT%"
rmdir /s /q "%TMP_DIR%"
echo Done.
