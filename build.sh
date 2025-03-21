#!/bin/bash

EMCC_DEBUG=1 emcc -O3 process_flac.cpp -o process_flac.js \
    -s USE_PTHREADS=1 \
    -s EXPORTED_FUNCTIONS='["_processFlac", "_malloc","_free", "_create_buffer"]' \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "addFunction", "wasmMemory"]' \
    -s NO_DISABLE_EXCEPTION_CATCHING \
    -s ASYNCIFY \
    -I/Users/atmta/Projects/converter/flac-1.3.4/include \
    -L/Users/atmta/Projects/converter/flac-1.3.4/src/libFLAC/.libs -lFLAC \
    -I/Users/atmta/Projects/converter/lame-3.100/include \
    -L/Users/atmta/Projects/converter/lame-3.100/libmp3lame/.libs -lmp3lame \
    -sALLOW_TABLE_GROWTH -sASSERTIONS -sALLOW_MEMORY_GROWTH -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$removeFunction' \
    --exclude-file /Users/atmta/Projects/converter/flac/src/libFLAC/.libs/libFLAC.dylib \
    --exclude-file /Users/atmta/Projects/converter/lame-3.100/libmp3lame/.libs/libmp3lame.dylib \
    --post-js ./script.js
