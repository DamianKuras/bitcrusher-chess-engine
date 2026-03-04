@echo off
cd /d "%~dp0.."

echo Formatting C++ source files...
FOR /R src %%f IN (*.cpp *.h) DO clang-format -i "%%f"
FOR /R tests %%f IN (*.cpp *.h) DO clang-format -i "%%f"
FOR /R benchmarks %%f IN (*.cpp *.h) DO clang-format -i "%%f"
echo Formatting complete.
