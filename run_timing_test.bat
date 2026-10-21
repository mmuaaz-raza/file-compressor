@echo off
echo Compiling test_timing.cpp...
g++ -std=c++17 -O2 -Wall ^
    test_timing.cpp ^
    src/utils.cpp ^
    src/priority_queue.cpp ^
    src/lzss.cpp ^
    src/huffman.cpp ^
    src/huffman2.cpp ^
    src/deflate.cpp ^
    src/compressor_interface.cpp ^
    -o test_timing.exe

if %ERRORLEVEL%==0 (
    echo.
    echo Build succeeded! Running test_timing.exe...
    echo.
    test_timing.exe
) else (
    echo.
    echo Build FAILED!
)
pause
