@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0.."

SET CLANG_FORMAT=clang-format
WHERE clang-format >nul 2>&1
IF ERRORLEVEL 1 (
    FOR /f "usebackq tokens=*" %%i IN (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath ^| more`) DO SET VS_PATH=%%i
    SET CLANG_FORMAT=!VS_PATH!\VC\Tools\Llvm\x64\bin\clang-format.exe
)

echo Formatting C++ source files...
FOR /R src %%f IN (*.cpp *.h *.hpp) DO "!CLANG_FORMAT!" -i "%%f"
FOR /R tests %%f IN (*.cpp *.h *.hpp) DO "!CLANG_FORMAT!" -i "%%f"
FOR /R benchmarks %%f IN (*.cpp *.h *.hpp) DO "!CLANG_FORMAT!" -i "%%f"
echo Formatting complete.
