@echo off
cd /d "%~dp0.."
echo Cleaning workspace...
if exist "bin\"   rmdir /s /q "bin"
if exist "build\" rmdir /s /q "build"
echo Workspace clean.
