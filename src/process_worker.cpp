#include <iostream>
#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#include <string>
#include <stdlib.h>
#include "../libraries/lame-3.100/include/lame.h"
#include "../libraries/flac-1.3.4/include/FLAC/stream_decoder.h"
#include "callback_types.h" // Include the new header file

// Structure to hold PCM buffer data
PCMBuffer pcmBuffer;

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
    emscripten_log(EM_LOG_CONSOLE, "Decoded chunk: %d samples, %d channels", blockSize, channels);

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

extern "C"
{

    // Function to create a buffer
    EMSCRIPTEN_KEEPALIVE
    uint8_t *createBuffer(int dataSize)
    {
        emscripten_log(EM_LOG_CONSOLE, "CREATING POINTER, DATA LENGTH: %d", dataSize);
        return static_cast<uint8_t *>(malloc(dataSize * sizeof(uint8_t)));
    }

    EMSCRIPTEN_KEEPALIVE
    void processFlac(uint8_t *dataPointer, int length)
    {
        emscripten_log(EM_LOG_CONSOLE, "processFlac called with dataPointer: %p, length: %d", dataPointer, length);

        if (!dataPointer || length <= 0)
        {
            emscripten_log(EM_LOG_ERROR, "Invalid dataPointer or length");
            return;
        }

        // Log the received data for debugging
        emscripten_log(EM_LOG_CONSOLE, "Received dataPointer: %p, length: %d", dataPointer, length);

        // Process the FLAC data as before
        std::vector<uint8_t> flacData(dataPointer, dataPointer + length);
        FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();

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

        if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
        {
            emscripten_log(EM_LOG_ERROR, "Failed to initialize FLAC decoder");
            FLAC__stream_decoder_delete(decoder);
            return;
        }

        if (!FLAC__stream_decoder_process_until_end_of_stream(decoder))
        {
            emscripten_log(EM_LOG_ERROR, "Failed to process FLAC stream");
        }

        FLAC__stream_decoder_finish(decoder);
        FLAC__stream_decoder_delete(decoder);

        emscripten_log(EM_LOG_CONSOLE, "Finished processing FLAC stream");
    }
}
