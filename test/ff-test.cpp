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

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

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

    AVPacket *pkt;
    AVFrame *frame;
    int buf_size;
    uint8_t *buf;
    int buf_offset;
    
    TSDLAudioPlay *AudioPlay;
    audio_params_t audio_params;
    bool playstarted = false;
    int got_frame;
    
    struct SwrContext *swr_ctx;

    if (argc < 2)
    {
        printf("no file input\n");
        return -ENAVAIL;
    }
    filename = argv[1];

    signal(SIGINT, IntHandler);
    av_register_all();
    avformat_network_init();

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

    /*CodecCtx = avcodec_alloc_context3(NULL);
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
    }*/
    CodecCtx = FormatCtx->streams[audio_idx]->codec;

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

    pkt = (AVPacket *) av_malloc(sizeof(AVPacket));
    frame = av_frame_alloc();

    av_init_packet(pkt);

    audio_params.format = SND_PCM_FORMAT_S16_LE; //CodecCtx->sample_fmt == AV_SAMPLE_FMT_S16 ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_S16_LE;
    audio_params.channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    audio_params.sample_rate = CodecCtx->sample_rate;
    audio_params.samples = CodecCtx->frame_size > 0 ? CodecCtx->frame_size : 1024;
    AudioPlay = new TSDLAudioPlay(PLAY_DEVICE_NAME);
    
    if (AudioPlay->Open(&audio_params) < 0)
    {
        printf("open play device failed \n");
        goto CODEC_ERR;
    }

    printf("samples: %d \n", audio_params.samples);
    buf_size = av_samples_get_buffer_size(NULL, AudioPlay->Channels(), audio_params.samples, AV_SAMPLE_FMT_S16, 1);
    buf = (uint8_t *) av_malloc(buf_size);

    swr_ctx = swr_alloc();
    swr_ctx = swr_alloc_set_opts(swr_ctx, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, AudioPlay->SampleRate(),
        av_get_default_channel_layout(CodecCtx->channels), CodecCtx->sample_fmt, CodecCtx->sample_rate, 0, NULL);
    swr_init(swr_ctx);

    printf("buf size: %d samples: %d\n", buf_size, audio_params.samples);

    if (! playstarted)
    {
        AudioPlay->Pause(0);
        playstarted = true;
    }

    printf("audio play buffer size: %d \n", AudioPlay->BufferSize());

    while(KeepRunning)
    {
        ret = av_read_frame(FormatCtx, pkt);
        if (ret < 0)
        {
            printf("av_read_frame err: %d \n", ret);
            break;
        }

        if (pkt->stream_index != audio_idx)
        {
            printf("frame is not audio stream \n");
            av_free_packet(pkt);
            continue;
        }
        
        #if 0
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

        ret = avcodec_decode_audio4(CodecCtx, frame, &got_frame, pkt);
        if (ret < 0)
        {
            printf("avcodec_decode_audio4 err: %d \n", ret);
        }

        int data_size = buf_size;
        if (got_frame)
        {
            ret = swr_convert(swr_ctx, &buf, buf_size, (const uint8_t **)frame->data, frame->nb_samples);
            buf_offset = 0;
            printf("pkt: %d frame: %d %d \n", pkt->size, frame->nb_samples, 
                av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, CodecCtx->sample_fmt, 1));
            while (data_size > 0)
            {
                ret = AudioPlay->Write(buf + buf_offset, data_size);
                if (ret != data_size)
                {
                    SDL_Delay(10);
                    // break;
                }

                buf_offset += ret;
                data_size -= ret;
            }
        }

        av_free_packet(pkt);
    }

    usleep(4000 * 1000);

    delete AudioPlay;
    swr_free(&swr_ctx);
    av_frame_free(&frame);
CODEC_ERR:
    avcodec_free_context(&CodecCtx);
FORMAT_ERR:
    avformat_free_context(FormatCtx);

    return ret;
}

