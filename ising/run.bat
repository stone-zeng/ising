@echo off

set CONFIGURATION=Release
set NAME=ising-run.exe
set PROFILE=.\settings.json 

.\x64\%CONFIGURATION%\%NAME% -s %PROFILE%
