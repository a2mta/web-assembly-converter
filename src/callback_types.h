#ifndef CALLBACK_TYPES_H
#define CALLBACK_TYPES_H

#include <cstdint>
#include <cstddef>
#include <vector>  // Add this include for std::vector

// Define a callback type for processing encoded audio data
typedef void (*CallbackType)(const uint8_t *data, size_t length);

struct PCMBuffer
{
    std::vector<int16_t> data;
    unsigned sampleRate;
    unsigned channels;
};

// Structure to pass arguments between threads
struct WorkerArgs
{
    CallbackType callback;
    uint8_t *dataPointer;
    size_t length;
};

#endif // CALLBACK_TYPES_H
