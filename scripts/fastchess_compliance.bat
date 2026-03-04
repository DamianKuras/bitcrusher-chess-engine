@echo off
cd /d "%~dp0.."

where fastchess >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: 'fastchess' could not be found.
    echo Please ensure it is installed and added to your system PATH.
    exit /b 1
)

fastchess --compliance "%CD%\bin\Release\Uci.exe"
