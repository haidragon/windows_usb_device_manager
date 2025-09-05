@echo off

REM Initialize Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

REM Compile the program
cl src/final_device_manager.cpp /link setupapi.lib

echo.
echo Compilation completed!
echo You can now run the program with: final_device_manager.exe