#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "ff_decoder.h"
#include "sdl_audio.h"
#include "SDL.h"

#define PLAY_DEVICE_NAME    "audiocodec"

using namespace HiCreation;

static volatile int KeepRunning = 1;

static void IntHandler(int dummy)
{
    printf("%s dummy: %d\n", __func__, dummy);
    KeepRunning = 0;
}

static int decode_interrupt_cb(void *ctx)
{
    printf("interrupt from avformat \n");
    return 0;
}

int main(int argc, char **argv)
{
    int ret;
    char *filename;

    int audio_idx;
    AVFormatContext *FormatCtx;
    AVCodecContext *CodecCtx;
    AVCodec *Codec;

    AVPacket pkt;
    AVFrame *frame;
    int data_size;
    uint8_t *data_buf;
    int buf_offset;
    
    TSDLAudioPlay *AudioPlay;
    audio_params_t audio_params;
    bool playstarted = false;
    int got_frame;

    if (argc < 2)
    {
        printf("no file input\n");
        return -ENAVAIL;
    }
    filename = argv[1];

    signal(SIGINT, IntHandler);
    av_register_all();

    FormatCtx = avformat_alloc_context();
    if (! FormatCtx)
    {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        return -ENOMEM;
    }

    // FormatCtx->interrupt_callback.callback = decode_interrupt_cb;
    // FormatCtx->interrupt_callback.opaque = NULL; // user data passed by

    ret = avformat_open_input(&FormatCtx, filename, NULL, NULL);
    if (ret < 0)
    {
        printf("avformat_open_input: %s err: %d\n", filename, ret);
        goto FORMAT_ERR;
    }
    ret = avformat_find_stream_info(FormatCtx, NULL);
    if (ret < 0)
    {
        printf("Error, avformat_find_stream_info: %d \n", ret);
        goto FORMAT_ERR;
    }

    av_dump_format(FormatCtx, 0, filename, 0);

    audio_idx = av_find_best_stream(FormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_idx < 0)
    {
        printf("Error, not found audio stream in %s\n", filename);
        goto FORMAT_ERR;
    }

    CodecCtx = avcodec_alloc_context3(NULL);
    if (! CodecCtx)
    {
        printf("avcodec_alloc_context3 err: %d\n", errno);
        ret = -ENOMEM;
        goto FORMAT_ERR;
    }

    ret = avcodec_parameters_to_context(CodecCtx, FormatCtx->streams[audio_idx]->codecpar);
    if (ret < 0)
    {
        printf("avcodec_parameters_to_context err: %d\n", ret);
        goto CODEC_ERR;
    }

    Codec = avcodec_find_decoder(CodecCtx->codec_id);
    if (! Codec)
    {
        printf("avcodec_find_decoder err: %d\n", errno);
        goto CODEC_ERR;
    }
    printf("Codec id: %d ctx codec id: %d \n", Codec->id, CodecCtx->codec_id);
    CodecCtx->codec_id = Codec->id;

    ret = avcodec_open2(CodecCtx, Codec, NULL);
    if (ret < 0)
    {
        printf("avcodec_open2 err: %d\n", ret);
        goto CODEC_ERR;
    }

    printf("audio rate: %d channels: %d fmt: %d \n", 
        CodecCtx->sample_rate, CodecCtx->channels, CodecCtx->sample_fmt);

    frame = av_frame_alloc();

    audio_params.format = CodecCtx->sample_fmt == AV_SAMPLE_FMT_S16 ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_S16_LE;
    audio_params.channels = CodecCtx->channels;
    audio_params.sample_rate = CodecCtx->sample_rate;
    AudioPlay = new TSDLAudioPlay(PLAY_DEVICE_NAME);
    
    if (AudioPlay->Open(&audio_params) < 0)
    {
        printf("open play device failed \n");
        goto CODEC_ERR;
    }

    if (! playstarted)
    {
        AudioPlay->Pause(0);
        playstarted = true;
    }

    printf("audio play buffer size: %d \n", AudioPlay->BufferSize());

    while(KeepRunning)
    {
        ret = av_read_frame(FormatCtx, &pkt);
        if (ret < 0)
        {
            printf("av_read_frame err: %d \n", ret);
            break;
        }
        
        #if 1
        avcodec_send_packet(CodecCtx, &pkt);
        do
        {
            ret = avcodec_receive_frame(CodecCtx, frame);
            if (ret >= 0)
            {
                data_size = av_samples_get_buffer_size(NULL, frame->channels,
                    frame->nb_samples, CodecCtx->sample_fmt, 1);
                data_buf = frame->data[0];
                
                buf_offset = 0;
                printf("data_size: %d \n", data_size);
                while (data_size > 0)
                {
                    ret = AudioPlay->Write(data_buf + buf_offset, data_size);
                    if (ret != data_size)
                    {
                        SDL_Delay(100);
                        // usleep(100);
                        // break;
                    }

                    buf_offset += ret;
                    data_size -= ret;
                }
            }
            else
            {
                printf("avcodec_receive_frame err: %d\n", ret);
                if (ret == AVERROR_EOF)
                {
                    avcodec_flush_buffers(CodecCtx);
                }
            }
        } while(ret != AVERROR(EAGAIN));

        av_packet_unref(&pkt);
        #endif

        /*ret = avcodec_decode_audio4(CodecCtx, frame, &got_frame, &pkt);
        if (ret < 0)
        {
            printf("avcodec_decode_audio4 err: %d \n", ret);
            continue;
        }

        if (got_frame)
        {
            data_size = av_samples_get_buffer_size(NULL, frame->channels,
                frame->nb_samples, CodecCtx->sample_fmt, 1);
            data_buf = frame->data[0];
            buf_offset = 0;
            printf("data_size: %d \n", data_size);
            while (data_size > 0)
            {
                ret = AudioPlay->Write(data_buf + buf_offset, data_size);
                if (ret != data_size)
                {
                    SDL_Delay(10);
                    // break;
                }

                buf_offset += ret;
                data_size -= ret;
            }

            if (! playstarted)
            {
                AudioPlay->Pause(0);
                playstarted = true;
            }
        }

        av_packet_unref(&pkt);*/
    }

    usleep(4000 * 1000);

    delete AudioPlay;
    av_frame_free(&frame);
CODEC_ERR:
    avcodec_free_context(&CodecCtx);
FORMAT_ERR:
    avformat_free_context(FormatCtx);

    return ret;
}

