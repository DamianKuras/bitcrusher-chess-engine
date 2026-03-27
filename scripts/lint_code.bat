@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0.."

SET CLANG_TIDY=clang-tidy
WHERE clang-tidy >nul 2>&1
IF ERRORLEVEL 1 (
    SET CLANG_TIDY=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-tidy.exe
)

if not exist "build\compile-commands\compile_commands.json" (
    echo Generating compile_commands.json...
    call scripts\create_compile_commands.bat
    if errorlevel 1 exit /b 1
)

SET TIDY_ARGS=-p build\compile-commands --extra-arg=-D_M_AMD64=100 --extra-arg=-D_WIN64 --extra-arg=-fms-extensions --extra-arg=-fms-compatibility

if not "%~1"=="" (
    echo Linting %~1...
    "!CLANG_TIDY!" !TIDY_ARGS! "%~1"
) else (
    echo Running clang-tidy on C++ source files...
    FOR /R src %%f IN (*.cpp *.h) DO "!CLANG_TIDY!" !TIDY_ARGS! "%%f"
    FOR /R tests %%f IN (*.cpp *.h) DO "!CLANG_TIDY!" !TIDY_ARGS! "%%f"
    FOR /R benchmarks %%f IN (*.cpp *.h) DO "!CLANG_TIDY!" !TIDY_ARGS! "%%f"
)
echo Linting complete.
