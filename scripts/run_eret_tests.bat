@echo off
call "%~dp0_setup_env.bat" || exit /b 1
cd /d "%~dp0.."
if not exist "build\tests\CMakeCache.txt" cmake --preset tests
if %errorlevel% neq 0 exit /b 1
cmake --build --preset tests-release
if %errorlevel% neq 0 exit /b 1
bin\Release\Tests.exe --gtest_filter=EretTests/*
