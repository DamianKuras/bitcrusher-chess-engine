@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0.."

SET CLANG_TIDY=clang-tidy
WHERE clang-tidy >nul 2>&1
IF ERRORLEVEL 1 (
    SET CLANG_TIDY=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-tidy.exe
)

echo Running clang-tidy on C++ source files...
if not exist "build\compile-commands\compile_commands.json" (
    echo Generating compile_commands.json...
    call scripts\create_compile_commands.bat
    if errorlevel 1 exit /b 1
)

FOR /R src %%f IN (*.cpp *.h) DO "!CLANG_TIDY!" -p build\compile-commands "%%f"
FOR /R tests %%f IN (*.cpp *.h) DO "!CLANG_TIDY!" -p build\compile-commands "%%f"
FOR /R benchmarks %%f IN (*.cpp *.h) DO "!CLANG_TIDY!" -p build\compile-commands "%%f"
echo Linting complete.
