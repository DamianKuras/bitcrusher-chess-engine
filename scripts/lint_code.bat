@echo off
cd /d "%~dp0.."

echo Running clang-tidy on C++ source files...
if not exist "compile_commands.json" (
    echo Generating compile_commands.json...
    call scripts\create_compile_commands.bat
)

FOR /R src %%f IN (*.cpp *.h) DO clang-tidy -p . "%%f"
FOR /R tests %%f IN (*.cpp *.h) DO clang-tidy -p . "%%f"
FOR /R benchmarks %%f IN (*.cpp *.h) DO clang-tidy -p . "%%f"
echo Linting complete.
