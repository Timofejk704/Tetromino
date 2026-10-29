@echo off
rem chcp 65001 > nul
echo project compilation...

g++ tetromino.cpp -o tetromino.exe -I"C:\msys64\mingw64\include\pdcurses" -L"C:\msys64\mingw64\lib" -lpdcurses -mwindows

if %errorlevel% equ 0 (
    echo [SUCCESFULL] Programm tetromino.exe was compilated!
    echo Launching...
    echo ----------------------------------------
    tetromino.exe
) else (
    echo [ERROR] Programm tetromino.exe wasn't compilated.
)
pause