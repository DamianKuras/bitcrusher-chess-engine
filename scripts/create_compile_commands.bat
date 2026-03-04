@echo off
cd /d "%~dp0.."
premake5 --with-tests --with-benchmarks --with-uci export-compile-commands
