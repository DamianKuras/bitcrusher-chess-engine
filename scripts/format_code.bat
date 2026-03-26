@echo off
cd /d "%~dp0.."

SET CLANG_FORMAT=clang-format
WHERE clang-format >nul 2>&1
IF ERRORLEVEL 1 (
    SET CLANG_FORMAT=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-format.exe
)

echo Formatting C++ source files...
FOR /R src %%f IN (*.cpp *.h *.hpp) DO "%CLANG_FORMAT%" -i "%%f"
FOR /R tests %%f IN (*.cpp *.h *.hpp) DO "%CLANG_FORMAT%" -i "%%f"
FOR /R benchmarks %%f IN (*.cpp *.h *.hpp) DO "%CLANG_FORMAT%" -i "%%f"
echo Formatting complete.
