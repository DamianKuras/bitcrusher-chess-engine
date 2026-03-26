@echo off
call "%~dp0_setup_env.bat" || exit /b 1
cd /d "%~dp0.."
if not exist "build\tests-no-bmi2\CMakeCache.txt" cmake --preset tests-no-bmi2
if %errorlevel% neq 0 exit /b 1
cmake --build --preset tests-no-bmi2-release
if %errorlevel% neq 0 exit /b 1
bin\Release\Tests.exe --gtest_filter=-*slow
