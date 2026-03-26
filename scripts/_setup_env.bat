@echo off
REM Sets up VS-bundled Ninja, CMake, and MSVC (cl.exe) for use with Ninja generator.
REM Safe to call multiple times -- skips steps already done.
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -property installationPath ^| more`) do set VS_PATH=%%i
if not defined VS_PATH ( echo Error: Visual Studio not found via vswhere. >&2 & exit /b 1 )

REM Add VS-bundled Ninja and CMake to PATH
where ninja >nul 2>nul || set PATH=%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%
where cmake >nul 2>nul || set PATH=%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%

REM Set up MSVC compiler environment (cl.exe, include/lib paths) so Ninja uses MSVC not GCC
if not defined VCINSTALLDIR (
    call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
    if %errorlevel% neq 0 ( echo Error: vcvarsall.bat failed. >&2 & exit /b 1 )
)
REM Fall back to VS-bundled vcpkg when VCPKG_ROOT is not set externally
if not defined VCPKG_ROOT set VCPKG_ROOT=%VS_PATH%\VC\vcpkg
exit /b 0
