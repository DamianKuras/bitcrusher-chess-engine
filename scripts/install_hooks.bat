@echo off
cd /d "%~dp0.."
git config core.hooksPath .githooks
echo Git hooks installed.
