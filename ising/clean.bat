@echo off

rmdir /S /Q "Debug\"
rmdir /S /Q "Release\"
rmdir /S /Q "x64\Debug\"
rmdir /S /Q "x64\Release\"
rmdir /S /Q "x64\"

rmdir /S /Q "core\Debug\"
rmdir /S /Q "core\Release\"
rmdir /S /Q "core\x64\Debug\"
rmdir /S /Q "core\x64\Release\"
rmdir /S /Q "core\x64\"

rmdir /S /Q "run\Debug\"
rmdir /S /Q "run\Release\"
rmdir /S /Q "run\x64\Debug\"
rmdir /S /Q "run\x64\Release\"
rmdir /S /Q "run\x64\"

rmdir /S /Q "test\Debug\"
rmdir /S /Q "test\Release\"
rmdir /S /Q "test\x64\Debug\"
rmdir /S /Q "test\x64\Release\"
rmdir /S /Q "test\x64\"

del      /Q ".vs\ising\v15\ipch\*"
rmdir /S /Q ".vs\ising\v15\ipch\AutoPCH"
rmdir /S /Q ".vs\ising\v15\ipch\"
