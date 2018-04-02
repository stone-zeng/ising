@echo off

set CONFIGURATION=Release
set NAME=ising-exact.exe
set RESULT=result.csv

.\x64\%CONFIGURATION%\%NAME%>%RESULT%
