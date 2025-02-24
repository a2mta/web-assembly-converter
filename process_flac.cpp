#include <iostream>
#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#include <string>
#include <stdlib.h>
#include "lame-3.100/include/lame.h"
#include "flac-1.3.4/include/FLAC/stream_decoder.h"

// Define a callback type for processing the encoded MP3 data
typedef void (*CallbackType)(const uint8_t *data, size_t length);

// Structure to hold PCM buffer data
struct PCMBuffer
{
    std::vector<int16_t> data;
    unsigned sampleRate;
    unsigned channels;
};
PCMBuffer pcmBuffer;

// Global variables for state management
FLAC__StreamDecoder *decoder = nullptr;
std::vector<uint8_t> flacData;
CallbackType globalCallback;
lame_t lame;
size_t pcmOffset = 0;
std::vector<uint8_t> mp3Buffer;
std::vector<uint8_t> accumulatedMp3Data;
bool isProcessingFlac = true;

// Callback function for writing decoded FLAC data to PCM buffer
FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
    size_t blockSize = frame->header.blocksize;
    size_t channels = frame->header.channels;

    pcmBuffer.sampleRate = frame->header.sample_rate;
    pcmBuffer.channels = channels;

    for (size_t i = 0; i < blockSize; ++i)
    {
        for (size_t ch = 0; ch < channels; ++ch)
        {
            pcmBuffer.data.push_back(static_cast<int16_t>(buffer[ch][i]));
        }
    }

    // Calculate percentage processed
    std::vector<uint8_t> *flacData = static_cast<std::vector<uint8_t> *>(client_data);
    size_t totalSize = flacData->size() + pcmBuffer.data.size() * sizeof(int16_t);
    size_t processedSize = pcmBuffer.data.size() * sizeof(int16_t);
    float percentage = (static_cast<float>(processedSize) / totalSize) * 100;

    // Throttle callback calls
    static float lastPercentage = 0.0;
    if (percentage - lastPercentage >= 1.0 || percentage == 100.0)
    {
        lastPercentage = percentage;
        // Call JavaScript callback with percentage
        EM_ASM({
            if (typeof Module.onProgress === 'function') {
                Module.onProgress($0);
            } }, percentage);
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

// Callback function for handling FLAC metadata
void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        const FLAC__StreamMetadata_StreamInfo *streamInfo = &metadata->data.stream_info;
        emscripten_log(EM_LOG_CONSOLE, "FLAC metadata: total samples = %llu, sample rate = %u, channels = %u, bits per sample = %u",
                       streamInfo->total_samples, streamInfo->sample_rate, streamInfo->channels, streamInfo->bits_per_sample);
    }
}

// Callback function for handling FLAC decoder errors
void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    emscripten_log(EM_LOG_ERROR, "FLAC decoder error: %s", FLAC__StreamDecoderErrorStatusString[status]);
}

// Callback function for reading FLAC data
FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
    std::vector<uint8_t> *flacData = static_cast<std::vector<uint8_t> *>(client_data);

    if (*bytes > flacData->size())
    {
        *bytes = flacData->size();
    }

    if (*bytes > 0)
    {
        std::copy(flacData->begin(), flacData->begin() + *bytes, buffer);
        flacData->erase(flacData->begin(), flacData->begin() + *bytes);
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }
    else
    {
        emscripten_log(EM_LOG_CONSOLE, "STREAM HAS ENDED");
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
}

// Callback function for checking if end of FLAC data is reached
FLAC__bool eof_callback(const FLAC__StreamDecoder *decoder, void *client_data)
{
    std::vector<uint8_t> *flacData = static_cast<std::vector<uint8_t> *>(client_data);
    if (flacData->empty())
    {
        emscripten_log(EM_LOG_CONSOLE, "FLAC data is empty");
    }
    return flacData->empty();
}

// Function to process FLAC data in chunks
void processFlacChunk()
{
    if (isProcessingFlac)
    {
        if (!FLAC__stream_decoder_process_single(decoder) || FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
        {
            emscripten_log(EM_LOG_CONSOLE, "Finished processing FLAC stream");
            FLAC__stream_decoder_finish(decoder);
            FLAC__stream_decoder_delete(decoder);
            decoder = nullptr;

            if (pcmBuffer.data.empty())
            {
                emscripten_log(EM_LOG_ERROR, "PCM buffer is empty");
                emscripten_cancel_main_loop();
                return;
            }

            // Initialize LAME encoder
            emscripten_log(EM_LOG_CONSOLE, "Initializing LAME encoder");
            lame = lame_init();
            lame_set_in_samplerate(lame, pcmBuffer.sampleRate);
            lame_set_num_channels(lame, pcmBuffer.channels);
            lame_set_quality(lame, 5);
            lame_init_params(lame);

            mp3Buffer.resize(1024 * 1024); // 1MB buffer
            pcmOffset = 0;
            accumulatedMp3Data.clear();
            isProcessingFlac = false;
        }
    }
    else
    {
        // Encode PCM to MP3 in chunks
        size_t remaining = pcmBuffer.data.size() * sizeof(int16_t) - pcmOffset;
        size_t toEncode = std::min(mp3Buffer.size(), remaining);
        int mp3Size = lame_encode_buffer_interleaved(lame, pcmBuffer.data.data() + pcmOffset / sizeof(int16_t), toEncode / sizeof(int16_t) / pcmBuffer.channels, mp3Buffer.data(), mp3Buffer.size());

        if (mp3Size > 0)
        {
            accumulatedMp3Data.insert(accumulatedMp3Data.end(), mp3Buffer.begin(), mp3Buffer.begin() + mp3Size);
        }

        pcmOffset += toEncode;

        if (pcmOffset >= pcmBuffer.data.size() * sizeof(int16_t))
        {
            mp3Size = lame_encode_flush(lame, mp3Buffer.data(), mp3Buffer.size());
            if (mp3Size > 0)
            {
                accumulatedMp3Data.insert(accumulatedMp3Data.end(), mp3Buffer.begin(), mp3Buffer.begin() + mp3Size);
            }

            lame_close(lame);
            emscripten_log(EM_LOG_CONSOLE, "MP3 encoding completed.");

            if (globalCallback)
            {
                globalCallback(accumulatedMp3Data.data(), accumulatedMp3Data.size());
            }

            emscripten_cancel_main_loop();
        }
    }
}

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

    // Function to process FLAC data and convert it to MP3
    EMSCRIPTEN_KEEPALIVE
    void processFlac(const uint8_t *dataPointer, size_t length, CallbackType callback)
    {
        // Initialize FLAC decoder
        decoder = FLAC__stream_decoder_new();

        flacData.assign(dataPointer, dataPointer + length);
        globalCallback = callback;

        // Set up FLAC decoder with callbacks
        FLAC__stream_decoder_set_md5_checking(decoder, true);
        FLAC__StreamDecoderInitStatus initStatus = FLAC__stream_decoder_init_stream(
            decoder,
            read_callback,
            nullptr,
            nullptr,
            nullptr,
            eof_callback,
            write_callback,
            metadata_callback,
            error_callback,
            &flacData);

        emscripten_log(EM_LOG_CONSOLE, "DAT POINTER: %d, DATA LENGTH: %d", dataPointer, length);

        if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
        {
            emscripten_log(EM_LOG_ERROR, "Failed to initialize FLAC decoder: %s", FLAC__StreamDecoderInitStatusString[initStatus]);
            FLAC__stream_decoder_delete(decoder);
            return;
        }

        isProcessingFlac = true;
        emscripten_set_main_loop(processFlacChunk, 0, 1);
    }
}
