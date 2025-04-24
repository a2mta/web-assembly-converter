#include <iostream>
#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#include <emscripten/wasm_worker.h>
#include <string>
#include <stdlib.h>
#include "../libraries/lame-3.100/include/lame.h"
#include "../libraries/flac-1.3.4/include/FLAC/stream_decoder.h"
#include "callback_types.h" // Include the new header file

PCMBuffer pcmBuffer;
WorkerArgs workerArgs;

// Extern "C" block to expose the function to JavaScript
extern "C"
{
    // Function to create a buffer
    EMSCRIPTEN_KEEPALIVE
    uint8_t *create_buffer(int dataSize)
    {
        emscripten_log(EM_LOG_CONSOLE, "CREATING POINTER, DATA LENGTH: %d", dataSize);
        return static_cast<uint8_t *>(malloc(dataSize * sizeof(uint8_t)));
    }

    void worker_callback(char *data, int size, void *arg)
    {
        emscripten_log(EM_LOG_CONSOLE, "WORKER CALLBACK with data size: %d", size);

        if (data && size > 0)
        {
            // Extract the arguments we stored before
            WorkerArgs *args = static_cast<WorkerArgs *>(arg);

            if (args && args->callback)
            {
                char *mp3Copy = static_cast<char *>(malloc(size));
                if (mp3Copy)
                {
                    // Copy the data to the new buffer
                    memcpy(mp3Copy, data, size);

                    // Call the callback with our copied data
                    args->callback(reinterpret_cast<const uint8_t *>(mp3Copy), size);

                    // Note: mp3Copy is now owned by the JavaScript side
                }
            }
        }
        else
        {
            emscripten_log(EM_LOG_ERROR, "Worker returned empty or invalid data");
        }
    }

    // Function to process FLAC data and convert it to MP3
    EMSCRIPTEN_KEEPALIVE
    void processFlac(worker_handle worker, const uint8_t *dataPointer, size_t length, CallbackType callback)
    {
        emscripten_log(EM_LOG_CONSOLE, "Worker handle: %p", worker);
        emscripten_log(EM_LOG_CONSOLE, "Data pointer: %p", dataPointer);
        emscripten_log(EM_LOG_CONSOLE, "Data length: %zu", length);
        emscripten_log(EM_LOG_CONSOLE, "Callback address: %p", reinterpret_cast<void *>(callback));

        // Store the callback for later use
        workerArgs.callback = callback;
        workerArgs.dataPointer = const_cast<uint8_t *>(dataPointer);
        workerArgs.length = length;

        // Use the provided worker
        emscripten_call_worker(worker, "processWorker",
                               reinterpret_cast<char *>(const_cast<uint8_t *>(dataPointer)),
                               length,
                               worker_callback,
                               &workerArgs);
    }
}
