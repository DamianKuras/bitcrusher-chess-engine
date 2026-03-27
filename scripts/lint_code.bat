@REM usage: scripts\lint_code.bat src\engine\include\transposition_table.hpp
@echo off
setlocal enabledelayedexpansion

rem Go to repo root (scripts/..)
cd /d "%~dp0.."

rem 1) Prefer VS LLVM, then PATH, and show version
set "CLANG_TIDY="
if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-tidy.exe" (
    set "CLANG_TIDY=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-tidy.exe"
) else (
    where clang-tidy >nul 2>&1 && set "CLANG_TIDY=clang-tidy"
)

if not defined CLANG_TIDY (
    echo ERROR: clang-tidy not found in VS LLVM or PATH.
    exit /b 1
)

echo Using "%CLANG_TIDY%"
"%CLANG_TIDY%" --version
echo.

rem 2) Ensure compile_commands.json exists
if not exist "build\compile-commands\compile_commands.json" (
    echo Generating compile_commands.json...
    call scripts\create_compile_commands.bat
    if errorlevel 1 exit /b 1
)

rem 3) Common args to reuse
set "TIDY_PROJECT=-p build\compile-commands"
set "TIDY_EXTRA=--extra-arg=-D_M_AMD64=100 --extra-arg=-D_WIN64 --extra-arg=-fms-extensions --extra-arg=-fms-compatibility --extra-arg=/std:c++latest"
rem Optional: restrict or expand checks via .clang-tidy instead of CLI
rem set "TIDY_CHECKS=-checks=*-modernize-use-auto"
rem set "TIDY_HEADER_FILTER=-header-filter=src\\.*"

rem 4) If a file passed, run on that; otherwise run via run-clang-tidy if available
if not "%~1"=="" (
    echo Linting %~1...
    "%CLANG_TIDY%" %TIDY_PROJECT% %TIDY_EXTRA% %TIDY_CHECKS% %TIDY_HEADER_FILTER% "%~1"
) else (
    rem Prefer run-clang-tidy on the compilation database (parallel and respects compile_commands)
    where run-clang-tidy >nul 2>&1
    if not errorlevel 1 (
        echo Running run-clang-tidy on compilation database...
        run-clang-tidy %TIDY_PROJECT% %TIDY_CHECKS% %TIDY_HEADER_FILTER% -j %NUMBER_OF_PROCESSORS%
    ) else (
        echo run-clang-tidy not found, falling back to manual file scan...
        echo Running clang-tidy on C++ source files...
        for /R src %%f in (*.cpp *.hpp) do "%CLANG_TIDY%" %TIDY_PROJECT% %TIDY_EXTRA% %TIDY_CHECKS% %TIDY_HEADER_FILTER% "%%f"
        for /R tests %%f in (*.cpp *.hpp) do "%CLANG_TIDY%" %TIDY_PROJECT% %TIDY_EXTRA% %TIDY_CHECKS% %TIDY_HEADER_FILTER% "%%f"
        for /R benchmarks %%f in (*.cpp *.hpp) do "%CLANG_TIDY%" %TIDY_PROJECT% %TIDY_EXTRA% %TIDY_CHECKS% %TIDY_HEADER_FILTER% "%%f"
    )
)

echo Linting complete.
endlocal