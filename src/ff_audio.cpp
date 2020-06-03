#include "ff_audio.h"

#define PLAY_DEVICE_NAME    "audiocodec"

using namespace HiCreation;

static int RefCount = 0;
static FFAudioPlayer *FPlayerInst;

FFAudioPlayer* FFAudioPlayer::InstRef()
{
    if (! FPlayerInst)
    {
        FPlayerInst = new FFAudioPlayer();
        FPlayerInst->Init();
    }
    if (FPlayerInst)
    {
        RefCount++;
        return FPlayerInst;
    }
    else 
        return NULL;
}

void FFAudioPlayer::Release()
{
    if (RefCount <= 0)
        return;
    RefCount--;
    if (RefCount == 0)
    {
        delete FPlayerInst;
        FPlayerInst = NULL;
    }
}

FFAudioPlayer::FFAudioPlayer():
    FLoop(1), FState(UNKOWN), FAudioIdx(-1), FThreadDone(true),
    FFormatCtx(NULL), FCodecCtx(NULL),
    FSwrCtx(NULL), FBuf(NULL), FBufSize(0), FBufReaded(0), FFrame(NULL),
    FAudioMixer(TAudioMixer::InstRef()), FAudioSink(NULL), FPausedByRace(false)
{
}

FFAudioPlayer::~FFAudioPlayer()
{
    Stop();
    TAudioMixer::Release();
}

int FFAudioPlayer::Init()
{
    if (FState != UNKOWN)
        return 0;

    av_register_all();
    avformat_network_init();

    FState = STOPPED;
    return 0;
}

int FFAudioPlayer::Load(const char *file)
{
    int ret;
    AVCodec *Codec;
    if (FState != STOPPED || FState == LOADING)
        return -EBUSY;

    FState = LOADING;
    FFormatCtx = avformat_alloc_context();
    if (! FFormatCtx)
    {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        return -ENOMEM;
    }

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

    FAudioIdx = av_find_best_stream(FFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (FAudioIdx < 0)
    {
        printf("Error, not found audio stream\n");
        return -EINVAL;
    }

    FCodecCtx = FFormatCtx->streams[FAudioIdx]->codec;
    Codec = avcodec_find_decoder(FCodecCtx->codec_id);
    if (! Codec)
    {
        printf("avcodec_find_decoder err: %d\n", errno);
        return -EINVAL;
    }
    FCodecCtx->codec_id = Codec->id;

    ret = avcodec_open2(FCodecCtx, Codec, NULL);
    if (ret < 0)
    {
        printf("avcodec_open2 err: %d\n", ret);
        return -EINVAL;
    }

    FAudioParams.format = SND_PCM_FORMAT_S16_LE;
    FAudioParams.channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    FAudioParams.sample_rate = FCodecCtx->sample_rate;
    FAudioParams.samples = FCodecCtx->frame_size > 0 ? FCodecCtx->frame_size : 1024;
    
    if (FAudioMixer->Open(&FAudioParams) < 0)
    {
        printf("Audio play dev open err!\n");
        return -ENODEV;
    }

    av_init_packet(&FPkt);
    FFrame = av_frame_alloc();

    FBufSize = av_samples_get_buffer_size(NULL, FAudioParams.channels, FAudioParams.samples, AV_SAMPLE_FMT_S16, 1);
    FBuf = (uint8_t *) av_malloc(FBufSize);

    FSwrCtx = swr_alloc();
    FSwrCtx = swr_alloc_set_opts(FSwrCtx, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, FAudioParams.sample_rate,
        av_get_default_channel_layout(FCodecCtx->channels), FCodecCtx->sample_fmt, FCodecCtx->sample_rate, 0, NULL);
    swr_init(FSwrCtx);
    FState = LOADED;
    return 0;
}

int FFAudioPlayer::Rewind()
{
    if (IsStopped())
        return Play();

    Pause();
    avformat_seek_file(FFormatCtx, -1, INT64_MIN, 
        FFormatCtx->streams[FAudioIdx]->start_time, INT64_MAX, 0);
    return Play();
}

int FFAudioPlayer::Play(uint8_t loop)
{
    if (FState == RUNNING)
        return 0;
    if (FState != LOADED && FState != PAUSED)
        return -1;
    if (! FFormatCtx)
    {
        printf("already stopped, you need load file first!\n");
        return -1;
    }
    FState = RUNNING;
    if (loop >= 1)
        FLoop = loop;
    else 
        FLoop = 1;
    TThread::Start();
    usleep(20 * 1000);
    FAudioMixer->Add(this);
    FAudioMixer->Pause(this, 0);
}

int FFAudioPlayer::Pause()
{
    if (FState == RUNNING)
    {
        FState = PAUSED;
        FAudioMixer->Pause(this, 1);
        SetTerminating();
    }
    return 0;
}

int FFAudioPlayer::Stop()
{
    if (FState == STOPPED)
        return 0;
    if (FState == STOPPING)
        return -EBUSY;

    FState = STOPPING;
    SetTerminating();
    while (! FThreadDone)
        usleep(1000);

    FAudioMixer->Remove(this);
    if (FSwrCtx)
        swr_free(&FSwrCtx);
    if (FCodecCtx)
        avcodec_close(FCodecCtx);
    if (FFormatCtx)
        avformat_close_input(&FFormatCtx);
    if (FFrame)
        av_frame_free(&FFrame);
    if (FBuf)
    {
        av_free(FBuf);
        FBuf = NULL;
        FBufSize = 0;
    }

    FFormatCtx = NULL;
    FState = STOPPED;
}

int FFAudioPlayer::AddOrUpdateSink(IAudioSink *sink)
{
    FAudioSink = sink;
    if (FPausedByRace)
    {
        if (IsPaused())
            Play();
        FPausedByRace = false;
    }
}

void FFAudioPlayer::RemoveSink(IAudioSink *sink)
{
    if (FAudioSink == sink)
    {
        FAudioSink = NULL;
        if (IsRunning())
        {
            Pause();
            FPausedByRace = true;
        }
    }
}

void FFAudioPlayer::Execute(void)
{
    int ret;
    int got_frame;
    int writing;
    int writed;
    AVFrame *frame = FFrame;
    AVPacket *pkt = &FPkt;
    int64_t stream_start_time = FFormatCtx->streams[FAudioIdx]->start_time;
    FThreadDone = false;

    while (FState == RUNNING)
    {
        while (FBufSize - FBufReaded > 0 && FState == RUNNING)
        {
            writing = FBufSize - FBufReaded;
            writed = FAudioMixer->Write(FBuf + FBufReaded, writing);
            FBufReaded += writed;
            if (writed != writing && FState == RUNNING)
                usleep(10 * 1000);
        }
        if (FState != RUNNING)
            break;

        ret = av_read_frame(FFormatCtx, pkt);
        if (ret < 0)
        {
            printf("av_read_frame err: %d \n", ret);
            FLoop --;
            if (FLoop > 0)
            {
                ret = avformat_seek_file(FFormatCtx, -1, INT64_MIN, stream_start_time, INT64_MAX, 0);
                printf("loop play: %d ret: %d \n", FLoop, ret);
                continue;
            }
            else 
                break;
        }

        if (pkt->stream_index != FAudioIdx)
        {
            printf("frame is not audio stream \n");
            av_free_packet(pkt);
            continue;
        }

        ret = avcodec_decode_audio4(FCodecCtx, frame, &got_frame, pkt);
        if (ret < 0)
        {
            printf("avcodec_decode_audio4 err: %d \n", ret);
        }

        if (got_frame)
        {
            FBufReaded = 0;
            ret = swr_convert(FSwrCtx, &FBuf, FBufSize, (const uint8_t **)frame->data, frame->nb_samples);
            // printf("pkt: %d frame: %d %d \n", pkt->size, frame->nb_samples, 
                // av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, FCodecCtx->sample_fmt, 1));
        }
        av_free_packet(pkt);
    }
    FThreadDone = true;
    if (FState == RUNNING)
        Stop();
}
