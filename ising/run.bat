@echo off

set CONFIGURATION=Release
REM set NAME=ising-run.exe
set NAME=ising-lattice-data.exe
set PROFILE=.\settings.json

.\x64\%CONFIGURATION%\%NAME% -s %PROFILE%
