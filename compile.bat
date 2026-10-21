@echo off
g++ -std=c++17 -O2 -Wall ^
    main.cpp ^
    src/utils.cpp ^
    src/priority_queue.cpp ^
    src/lzss.cpp ^
    src/huffman.cpp ^
    src/huffman2.cpp ^
    src/deflate.cpp ^
    src/compressor_interface.cpp ^
    -lpsapi ^
    -o deflate_app.exe
if %ERRORLEVEL%==0 (
    echo Build succeeded: deflate_app.exe
) else (
    echo Build FAILED.
)
