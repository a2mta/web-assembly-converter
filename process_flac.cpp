#include <iostream>
#include <emscripten/emscripten.h>
#include <string>
#include <stdlib.h>
#include "lame-3.100/include/lame.h"
#include "flac/include/FLAC/stream_decoder.h"

typedef void (*CallbackType)(const uint8_t *data, size_t length);

struct PCMBuffer
{
    std::vector<int16_t> data;
    unsigned sampleRate;
    unsigned channels;
};

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
    PCMBuffer *pcmBuffer = static_cast<PCMBuffer *>(client_data);
    size_t blockSize = frame->header.blocksize;
    size_t channels = frame->header.channels;
    for (size_t i = 0; i < blockSize; ++i)
    {
        for (size_t ch = 0; ch < channels; ++ch)
        {
            pcmBuffer->data.push_back(buffer[ch][i]);
        }
    }
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

extern "C"
{
    EMSCRIPTEN_KEEPALIVE
    void processFlac(const uint8_t *data, size_t length, CallbackType callback)
    {
        // Initialize FLAC decoder
        FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
        PCMBuffer pcmBuffer;

        FLAC__stream_decoder_set_md5_checking(decoder, true);
        FLAC__StreamDecoderInitStatus initStatus = FLAC__stream_decoder_init_file(decoder, "temp.flac", write_callback, nullptr, nullptr, &pcmBuffer);
        if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
        {
            FLAC__stream_decoder_delete(decoder);
            return;
        }

        // Write binary data to a temporary file
        FILE *tempFile = fopen("temp.flac", "wb");
        fwrite(data, 1, length, tempFile);
        fclose(tempFile);

        FLAC__stream_decoder_process_until_end_of_stream(decoder);
        FLAC__stream_decoder_delete(decoder);

        if (pcmBuffer.data.empty())
        {
            return;
        }

        // Encode PCM to MP3 using LAME
        lame_t lame = lame_init();
        lame_set_in_samplerate(lame, pcmBuffer.sampleRate);
        lame_set_num_channels(lame, pcmBuffer.channels);
        lame_set_quality(lame, 5);
        lame_init_params(lame);

        size_t pcmSize = pcmBuffer.data.size() * sizeof(int16_t);
        std::vector<uint8_t> mp3Buffer(pcmSize); // Overestimate size
        int mp3Size = lame_encode_buffer_interleaved(lame, pcmBuffer.data.data(), pcmBuffer.data.size() / pcmBuffer.channels, mp3Buffer.data(), mp3Buffer.size());

        lame_close(lame);

        if (mp3Size > 0)
        {
            callback(mp3Buffer.data(), mp3Size);
        }
    }
}

