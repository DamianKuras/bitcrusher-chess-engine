@echo off
call "%~dp0_setup_env.bat" || exit /b 1
REM Generates compile_commands.json for clangd/IDE tooling.
REM Requires Ninja: cmake will use it for a single-config build that exports the DB.
cd /d "%~dp0.."
if exist "build\compile-commands" rmdir /s /q "build\compile-commands"
cmake -S . -B build\compile-commands -G Ninja ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
    -DBITCRUSHER_BUILD_UCI=ON ^
    -DBITCRUSHER_BUILD_TESTS=ON ^
    -DBITCRUSHER_BUILD_BENCHMARKS=ON ^
    -DBITCRUSHER_WITH_BMI2=ON
if %errorlevel% neq 0 exit /b 1
copy /y "build\compile-commands\compile_commands.json" "compile_commands.json"
echo compile_commands.json written to repo root.
