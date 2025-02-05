#include <iostream>
#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#include <string>
#include <stdlib.h>
#include "lame-3.100/include/lame.h"
#include "flac/include/FLAC/stream_decoder.h"
static FLAC__uint64 total_samples = 0;
static uint32_t bps = 0;
static uint32_t channels = 0;
// Define a callback type for processing the encoded MP3 data
typedef void (*CallbackType)(const uint8_t *data, size_t length);

// Structure to hold PCM buffer data
struct PCMBuffer
{
    std::vector<int16_t> data;
    unsigned sampleRate;
    unsigned channels;
};

// Callback function for writing decoded FLAC data to PCM buffer
FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
    const FLAC__uint32 total_size = (FLAC__uint32)(total_samples * channels * (bps / 8));
    channels = FLAC__stream_decoder_get_channels(decoder);
	bps = FLAC__stream_decoder_get_bits_per_sample(decoder);
    PCMBuffer *pcmBuffer = static_cast<PCMBuffer *>(client_data);
    size_t blockSize = frame->header.blocksize;
    size_t channels = frame->header.channels;

	// for(i = 0; i < frame->header.blocksize; i++) {
    for (size_t i = 0; i < frame->header.blocksize; ++i)
    {
        for (size_t ch = 0; ch < channels; ++ch)
        {
            pcmBuffer->data.push_back(buffer[ch][i]);
        }
    }

    emscripten_log(EM_LOG_CONSOLE, "Writing PCM data to buffer. Sample rate: %u, Channels: %u, Block size: %zu, BPS: %zu", pcmBuffer->sampleRate, channels, blockSize, bps);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

// Callback function for handling FLAC metadata
void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        const FLAC__StreamMetadata_StreamInfo *streamInfo = &metadata->data.stream_info;
        total_samples = streamInfo->total_samples;
        channels = streamInfo->channels;
        bps = streamInfo->bits_per_sample;
        emscripten_log(EM_LOG_CONSOLE, "FLAC metadata: total samples = %llu, sample rate = %u, channels = %u, bits per sample = %u",
                       total_samples, streamInfo->sample_rate, streamInfo->channels, streamInfo->bits_per_sample);
    }
}

// Callback function for handling FLAC decoder errors
void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    static int logCounter = 0;
    if (logCounter++ % 100 == 0) // Log every 100th call
    {
        emscripten_log(EM_LOG_ERROR, "FLAC decoder error: %s", FLAC__StreamDecoderErrorStatusString[status]);
    }
    if (status == FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC)
    {
        emscripten_log(EM_LOG_ERROR, "FLAC decoder lost sync. Possible corrupted data.");
    }
}

// Callback function for reading FLAC data
FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
    std::vector<uint8_t> *flacData = static_cast<std::vector<uint8_t> *>(client_data);
    static int logCounter = 0;

    if (*bytes > flacData->size())
    {
        *bytes = flacData->size();
    }
    if (*bytes > 0)
    {
        if (logCounter++ % 100 == 0) // Log every 100th call
        {
            emscripten_log(EM_LOG_CONSOLE, "Reading FLAC data: %zu bytes remaining", flacData->size());
        }
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

// Extern "C" block to expose the function to JavaScript
extern "C"
{
    // Function to process FLAC data and convert it to MP3
    EMSCRIPTEN_KEEPALIVE
    void processFlac(const uint8_t *data, size_t length, CallbackType callback)
    {
        // Initialize FLAC decoder
        FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();

        PCMBuffer pcmBuffer;
        std::vector<uint8_t> flacData(data, data + length);
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

        emscripten_log(EM_LOG_CONSOLE, "FLAC decoder init status: %d", initStatus);
        if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
        {
            emscripten_log(EM_LOG_ERROR, "Failed to initialize FLAC decoder: %s", FLAC__StreamDecoderInitStatusString[initStatus]);
            FLAC__stream_decoder_delete(decoder);
            return;
        }

        // Process the FLAC stream
        emscripten_log(EM_LOG_CONSOLE, "Processing FLAC stream");
        FLAC__stream_decoder_process_until_end_of_stream(decoder);
        emscripten_log(EM_LOG_CONSOLE, "Finished processing FLAC stream");

        FLAC__stream_decoder_delete(decoder);

        // if (pcmBuffer.data.empty())
        // {
        //     emscripten_log(EM_LOG_ERROR, "PCM buffer is empty");
        //     return;
        // }

        // // Encode PCM to MP3 using LAME
        // emscripten_log(EM_LOG_CONSOLE, "Initializing LAME encoder");
        // lame_t lame = lame_init();
        // lame_set_in_samplerate(lame, pcmBuffer.sampleRate);
        // lame_set_num_channels(lame, pcmBuffer.channels);
        // lame_set_quality(lame, 5);
        // lame_init_params(lame);

        // // Overestimate size of MP3 buffer
        // size_t pcmSize = pcmBuffer.data.size() * sizeof(int16_t);
        // std::vector<uint8_t> mp3Buffer(pcmSize);
        // int mp3Size = lame_encode_buffer_interleaved(lame, pcmBuffer.data.data(), pcmBuffer.data.size() / pcmBuffer.channels, mp3Buffer.data(), mp3Buffer.size());

        // lame_close(lame);
        // emscripten_log(EM_LOG_CONSOLE, "MP3 encoding completed. Size: %d bytes", mp3Size);
        // if (mp3Size > 0)
        // {
        //     callback(mp3Buffer.data(), mp3Size);
        // }
    }
}
