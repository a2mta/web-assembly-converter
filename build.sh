#!/bin/bash

# EMCC_DEBUG=1 emcc -O3 ./src/process_worker.cpp -o ./build/process_flac.js \
#     -s USE_PTHREADS=1 \
#     -s WASM_WORKERS=1 \
#     -s PTHREAD_POOL_SIZE=4 \
#     -s ASSERTIONS \
#     -s EXPORTED_FUNCTIONS='["_processFlac", "_malloc","_free", "_create_buffer"]' \
#     -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "addFunction", "wasmMemory"]' \
#     -s NO_DISABLE_EXCEPTION_CATCHING \
#     -s ASYNCIFY \
#     -I ./libraries/flac-1.3.4/include \
#     -L ./libraries/flac-1.3.4/src/libFLAC/.libs -lFLAC \
#     -I ./libraries/lame-3.100/include \
#     -L ./libraries/lame-3.100/libmp3lame/.libs -lmp3lame \
#     -sALLOW_TABLE_GROWTH -sASSERTIONS -sALLOW_MEMORY_GROWTH -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$removeFunction' \
#     --exclude-file ./libraries/flac/src/libFLAC/.libs/libFLAC.dylib \
#     --exclude-file ./libraries/lame-3.100/libmp3lame/.libs/libmp3lame.dylib \
#     --post-js ./src/script.js


EMCC_DEBUG=1 emcc -Os ./src/process_worker.cpp -o ./build/process_flac.wasm \
    -s EXPORTED_FUNCTIONS='["_processFlac", "_malloc","_free", "_createBuffer"]' \
    -I /Users/atmta/Projects/converter/libraries/flac-1.3.4/include \
    -L /Users/atmta/Projects/converter/libraries/flac-1.3.4/src/libFLAC/.libs -lFLAC \
    -I /Users/atmta/Projects/converter/libraries/lame-3.100/include \
    -L /Users/atmta/Projects/converter/libraries/lame-3.100/libmp3lame/.libs -lmp3lame \
    -s ALLOW_TABLE_GROWTH -s ALLOW_MEMORY_GROWTH  \
    --no-entry \

