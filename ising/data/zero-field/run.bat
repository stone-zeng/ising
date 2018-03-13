@echo off

set CONFIGURATION=Release
set NAME=ising-run.exe
set ISING=..\..\x64\%CONFIGURATION%\%NAME%

rem del free\*.csv
rem del periodic\*.csv

%ISING% -s "free\size4.json"        > "free\size4.csv"
%ISING% -s "free\size8.json"        > "free\size8.csv"
%ISING% -s "free\size16.json"       > "free\size16.csv"
%ISING% -s "free\size32.json"       > "free\size32.csv"
%ISING% -s "free\size64.json"       > "free\size64.csv"
%ISING% -s "free\size96.json"       > "free\size96.csv"
%ISING% -s "free\size128.json"      > "free\size128.csv"
%ISING% -s "free\size192.json"      > "free\size192.csv"
%ISING% -s "free\size256.json"      > "free\size256.csv"

%ISING% -s "periodic\size4.json"    > "periodic\size4.csv"
%ISING% -s "periodic\size8.json"    > "periodic\size8.csv"
%ISING% -s "periodic\size16.json"   > "periodic\size16.csv"
%ISING% -s "periodic\size32.json"   > "periodic\size32.csv"
%ISING% -s "periodic\size64.json"   > "periodic\size64.csv"
%ISING% -s "periodic\size96.json"   > "periodic\size96.csv"
%ISING% -s "periodic\size128.json"  > "periodic\size128.csv"
%ISING% -s "periodic\size192.json"  > "periodic\size192.csv"
%ISING% -s "periodic\size256.json"  > "periodic\size256.csv"
