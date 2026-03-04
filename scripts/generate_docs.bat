@echo off
cd /d "%~dp0.."

echo Generating documentation...
if exist "html\" rmdir /s /q "html"
if exist "latex\" rmdir /s /q "latex"
doxygen Doxyfile
echo Documentation generated in html/ and latex/
