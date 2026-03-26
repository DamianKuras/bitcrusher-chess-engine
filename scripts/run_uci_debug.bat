@echo off
call "%~dp0_setup_env.bat" || exit /b 1
cd /d "%~dp0.."
if not exist "build\uci\CMakeCache.txt" cmake --preset uci
if %errorlevel% neq 0 exit /b 1
cmake --build --preset uci-debug
if %errorlevel% neq 0 exit /b 1
bin\Debug\Uci.exe
