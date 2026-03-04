@echo off
cd /d "%~dp0.."

REM Generate Visual Studio Solution
premake5 vs2022 --with-tests --with-bmi2

REM Find MSBuild
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBUILD="%%i"
)
if not defined MSBUILD (
    echo Error: Could not find MSBuild.exe.
    exit /b 1
)

REM Build and run
cd build
%MSBUILD% BitcrusherChessEngine.sln /p:Configuration=Release /p:Platform=x64 /t:Clean
%MSBUILD% BitcrusherChessEngine.sln /p:Configuration=Release /p:Platform=x64
cd ..
bin\Release\Tests.exe
