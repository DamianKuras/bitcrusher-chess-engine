@echo off
cd /d "%~dp0.."

where cutechess-cli >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: 'cutechess-cli' could not be found.
    echo Please ensure it is installed and added to your system PATH.
    exit /b 1
)

where stockfish-windows-x86-64-avx2 >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: 'stockfish-windows-x86-64-avx2' could not be found.
    echo Please ensure the stockfish engine executable is in your system PATH or update this script with its exact name.
    exit /b 1
)

cutechess-cli -engine cmd=bin\Release\Uci.exe -engine cmd=stockfish-windows-x86-64-avx2 -each proto=uci tc=40/30 -rounds 1
