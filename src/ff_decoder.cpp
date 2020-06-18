#include "ff_decoder.h"

using namespace HiCreation;

int TAVFormat::Init()
{
    av_register_all();
    avformat_network_init();

    FFormatCtx = avformat_alloc_context();
    if (! FFormatCtx)
    {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        return -ENOMEM;
    }
    return 0;
}

void TAVFormat::UnInit()
{
    if (FFormatCtx)
        avformat_free_context(FFormatCtx);
}

int TAVFormat::Open(const char *file)
{
    int ret;
    ret = avformat_open_input(&FFormatCtx, file, NULL, NULL);
    if (ret < 0)
    {
        printf("avformat_open_input: %s err: %d\n", file, ret);
        return ret;
    }
    ret = avformat_find_stream_info(FFormatCtx, NULL);
    if (ret < 0)
    {
        printf("Error, avformat_find_stream_info: %d \n", ret);
        return ret;
    }
    av_dump_format(FFormatCtx, 0, file, 0);
    return 0;
}

int TAVFormat::Close()
{

}

FFDecoder* TAVFormat::Decoder(AVMediaType type)
{
    int ret;
    int stream_idx;
    AVCodecContext *CodecCtx;
    AVCodec *Codec;
    if (! FFormatCtx)
        return NULL;

    stream_idx = av_find_best_stream(FFormatCtx, type, -1, -1, NULL, 0);
    if (stream_idx < 0)
    {
        printf("Error, not found stream type: \n", type);
        return NULL;
    }

    CodecCtx = FFormatCtx->streams[stream_idx]->codec;
    Codec = avcodec_find_decoder(CodecCtx->codec_id);
    if (! Codec)
    {
        printf("avcodec_find_decoder err: %d\n", errno);
        return NULL;
    }
    CodecCtx->codec_id = Codec->id;

    ret = avcodec_open2(CodecCtx, Codec, NULL);
    if (ret < 0)
    {
        printf("avcodec_open2 err: %d\n", ret);
        return NULL;
    }

    switch (type)
    {
    case AVMEDIA_TYPE_AUDIO:
        return new FFAudioDecoder(this, CodecCtx, stream_idx);
    case AVMEDIA_TYPE_VIDEO:
        return NULL;
    case AVMEDIA_TYPE_SUBTITLE:
        return NULL;
    }
}

int TAVFormat::ReadFrame(int stream, AVPacket *pkt)
{
    int ret;
    ret = av_read_frame(FFormatCtx, pkt);
    if (ret < 0)
    {
        printf("av_read_frame err: %d \n", ret);
        return ret;
    }

    if (pkt->stream_index != stream)
    {
        printf("frame is not audio stream \n");
        av_free_packet(pkt);
        return -EAGAIN;
    }
    return 0;
}

int FFDecoder::Init()
{
    av_init_packet(&FPkt);
    return 0;
}

int FFAudioDecoder::Init()
{
    int ret;
    if ((ret = inherited::Init()) != 0)
        return ret;

    FAudioParams.format = SND_PCM_FORMAT_S16_LE; //CodecCtx->sample_fmt == AV_SAMPLE_FMT_S16 ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_S16_LE;
    FAudioParams.channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    FAudioParams.sample_rate = FCodecCtx->sample_rate;
    FAudioParams.samples = FCodecCtx->frame_size > 0 ? FCodecCtx->frame_size : 1024;

    return SetAudioParams(&FAudioParams);
}

int FFAudioDecoder::SetAudioParams(audio_params_t *params)
{
    int buf_size;
    buf_size = av_samples_get_buffer_size(NULL, params->channels, params->samples, AV_SAMPLE_FMT_S16, 1);
    if (FBufSize < buf_size)
    {
        if (FSwrBuf)
            av_free(FSwrBuf);
        FSwrBuf = (uint8_t *) av_malloc(buf_size);
        FBufSize = buf_size;
    }

    if (FSwrCtx)
        swr_free(&FSwrCtx);

    FSwrCtx = swr_alloc();
    FSwrCtx = swr_alloc_set_opts(FSwrCtx, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, params->sample_rate,
        av_get_default_channel_layout(FCodecCtx->channels), FCodecCtx->sample_fmt, FCodecCtx->sample_rate, 0, NULL);
    swr_init(FSwrCtx);
}

int FFAudioDecoder::DecodeFrame()
{
    int ret;
    int try_times = 5;
    while (try_times > 0)
    {
        ret = FAVFormat->ReadFrame(StreamIdx(), &FPkt);
        if (ret == 0)
        {
            avcodec_decode_audio4(FCodecCtx, &FFrame, &ret, &FPkt);
            if (ret)
            {
                ret = swr_convert(FSwrCtx, &FSwrBuf, FBufSize, (const uint8_t **)FFrame.data, FFrame.nb_samples);
                printf("pkt: %d frame: %d %d \n", FPkt.size, FFrame.nb_samples, 
                    av_samples_get_buffer_size(NULL, FFrame.channels, FFrame.nb_samples, FCodecCtx->sample_fmt, 1));
                return FBufSize;
            }
        }
        else if (ret != -EAGAIN)
            continue;
        else 
            return ret;

        av_free_packet(&FPkt);
        try_times--;
    }
    return -ETIMEDOUT;
}

int FFAudioDecoder::OnFillFrame(IAudioSink *sink, audio_frame_t *frame)
{
    int reading = frame->len;
    int decoded;
    int copy_len;
    while (reading > 0)
    {
        if (FBufReaded >= FBufSize)
        {
            decoded = DecodeFrame();
            if (decoded < 0)
            {
                decoded = 1024;
            }
            else if (decoded == 0)
            {
                break;
            }
            FBufReaded = 0;
        }
        copy_len = FBufSize - FBufReaded;
        if (copy_len > reading)
            copy_len = reading;
        memcpy(frame->buf + (frame->len - reading), FSwrBuf + FBufReaded, copy_len);
        FBufReaded += copy_len;
        reading -= copy_len;
    }
    return frame->len - reading;
}