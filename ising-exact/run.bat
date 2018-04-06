@echo off

set CONFIGURATION=Release
set NAME=ising-exact.exe
set ARGS=9 0.001
set RESULT=result.csv

.\x64\%CONFIGURATION%\%NAME% %ARGS%>%RESULT%
