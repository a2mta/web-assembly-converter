#include <iostream>
#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#include <string>
#include <stdlib.h>
#include "lame-3.100/include/lame.h"
#include "flac-1.3.4/include/FLAC/stream_decoder.h"
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
    EMSCRIPTEN_KEEPALIVE
    void processWorker(char *dataPointer, int length)
    {
        // Clear the PCM buffer for this new processing
        pcmBuffer.data.clear();
        pcmBuffer.sampleRate = 0;
        pcmBuffer.channels = 0;

        // Check if we received valid data
        if (!dataPointer || length <= 0)
        {
            emscripten_log(EM_LOG_ERROR, "Invalid data received in worker: pointer=%p, length=%d", dataPointer, length);
            return;
        }

        emscripten_log(EM_LOG_CONSOLE, "Worker received data: pointer=%p, length=%d", dataPointer, length);

        // Initialize FLAC decoder
        FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();

        std::vector<uint8_t> flacData(dataPointer, dataPointer + length);
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

        emscripten_log(EM_LOG_CONSOLE, "DATA POINTER: %d, DATA LENGTH: %d", dataPointer, length);

        if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
        {
            emscripten_log(EM_LOG_ERROR, "Failed to initialize FLAC decoder: %s", FLAC__StreamDecoderInitStatusString[initStatus]);
            FLAC__stream_decoder_delete(decoder);
            return;
        }

        if (!FLAC__stream_decoder_process_until_end_of_stream(decoder))
        {
            emscripten_log(EM_LOG_CONSOLE, "Error processing FLAC stream\n");
        }
        emscripten_log(EM_LOG_CONSOLE, "Finished processing FLAC stream");

        FLAC__stream_decoder_finish(decoder);
        FLAC__stream_decoder_delete(decoder);

        if (pcmBuffer.data.empty())
        {
            emscripten_log(EM_LOG_ERROR, "PCM buffer is empty");
            return;
        }

        // // Encode PCM to MP3 using LAME
        emscripten_log(EM_LOG_CONSOLE, "Initializing LAME encoder");
        lame_t lame = lame_init();
        lame_set_in_samplerate(lame, pcmBuffer.sampleRate);
        lame_set_num_channels(lame, pcmBuffer.channels);
        lame_set_quality(lame, 5);
        lame_init_params(lame);

        // Overestimate size of MP3 buffer
        size_t pcmSize = pcmBuffer.data.size() * sizeof(int16_t);
        std::vector<uint8_t> mp3Buffer(pcmSize);
        int mp3Size = lame_encode_buffer_interleaved(lame, pcmBuffer.data.data(), pcmBuffer.data.size() / pcmBuffer.channels, mp3Buffer.data(), mp3Buffer.size());

        lame_close(lame);
        emscripten_log(EM_LOG_CONSOLE, "MP3 encoding completed. Size: %d bytes", mp3Size);

        if (mp3Size > 0)
        {
            char *mp3Copy = static_cast<char *>(malloc(mp3Size));
            if (mp3Copy)
            {
                memcpy(mp3Copy, mp3Buffer.data(), mp3Size);
                emscripten_worker_respond(mp3Copy, mp3Size);
                free(mp3Copy); // Free memory if respond fails
            }
            else
            {
                const char *error_msg = "Failed to allocate memory for MP3 data";
                emscripten_worker_respond(const_cast<char *>(error_msg), strlen(error_msg));
            }
        }
        else
        {
            const char *error_msg = "MP3 encoding failed";
            emscripten_worker_respond(const_cast<char *>(error_msg), strlen(error_msg));
        }

        // Clear PCM buffer to free memory
        pcmBuffer.data.clear();
        pcmBuffer.data.shrink_to_fit();
        // Clear MP3 buffer to free memory
        mp3Buffer.clear();
        mp3Buffer.shrink_to_fit();
    }
}
