#include <stdio.h>
#include <string.h>

#include "sdl_audio.h"

#define DEFAULT_SDL_AUDIO_SAMPLES     1024
#define MAX_SDL_AUDIO_SAMPLES         8192
#define MAX_AUDIO_FRAME_SIZE          192000

using namespace HiCreation;

typedef struct format_map_t
{
    snd_pcm_format_t snd_format;
    SDL_AudioFormat sdl_format;
} format_map_t;

static format_map_t FormatMaps[] =
{
    {SND_PCM_FORMAT_S8,     AUDIO_S8},
    {SND_PCM_FORMAT_U8,     AUDIO_U8},
    {SND_PCM_FORMAT_S16,    AUDIO_S16},
    {SND_PCM_FORMAT_U16,    AUDIO_U16},
    {SND_PCM_FORMAT_S16_LE, AUDIO_S16LSB},
    {SND_PCM_FORMAT_S16_BE, AUDIO_S16MSB},
    {SND_PCM_FORMAT_U16_LE, AUDIO_U16LSB},
    {SND_PCM_FORMAT_U16_BE, AUDIO_U16MSB},
    {SND_PCM_FORMAT_S32,    AUDIO_S32},
    {SND_PCM_FORMAT_S32_LE, AUDIO_S32LSB},
    {SND_PCM_FORMAT_S32_BE, AUDIO_S32MSB}
};

SDL_AudioFormat TSDLAudioDev::ToSDLAudioFormat(snd_pcm_format_t format)
{
    int i;
    int map_count = sizeof(FormatMaps) / sizeof(format_map_t);
    for (i = 0; i < map_count; i++)
        if (FormatMaps[i].snd_format == format)
            return FormatMaps[i].sdl_format;

    printf("no map for snd format: %d\n", format);
    return AUDIO_S16SYS;
}

snd_pcm_format_t TSDLAudioDev::ToAudioFormat(SDL_AudioFormat format)
{
    int i;
    int map_count = sizeof(FormatMaps) / sizeof(format_map_t);
    for (i = 0; i < map_count; i++)
        if (FormatMaps[i].sdl_format == format)
            return FormatMaps[i].snd_format;
    
    printf("no map for sdl format: 0x%04x\n", format);
    return SND_PCM_FORMAT_UNKNOWN;
}

int TSDLAudioDev::GetDeviceIdx(const char *dev_name)
{
    int i;
    for (i = 0; i < TSDLAudioDev::Devices(FIsRecord); i++)
    {
        if (strncmp(dev_name, TSDLAudioDev::DeviceName(i, FIsRecord), strlen(dev_name)) == 0)
            return i;
    }
    printf("get %s dev: %s idx failed\n", FIsRecord ? "record" : "play", dev_name);
    return 0;
}

TSDLAudioDev::TSDLAudioDev(const char *dev_name, bool isrecord):
    FIsRecord(isrecord), FDevId(-1), FState(AUDIO_STATE_UNKOWN)
{
    if (TSDLAudioDev::Init() < 0)
        printf("sdl init failed\n");

    memset(&FWantedSpec, 0, sizeof(FWantedSpec));
    memset(&FObtainedSpec, 0, sizeof(FObtainedSpec));
    FDevIdx = GetDeviceIdx(dev_name);
}

TSDLAudioDev::~TSDLAudioDev()
{
    Close();
}

int TSDLAudioDev::Open(audio_params_t *params)
{
    if (FState != AUDIO_STATE_UNKOWN && FState != AUDIO_STATE_CLOSED)
        return -1;

    FWantedSpec.format = TSDLAudioDev::ToSDLAudioFormat(params->format);
    FWantedSpec.channels = params->channels;
    FWantedSpec.freq = params->sample_rate;
    FWantedSpec.samples = params->samples > 0 ? params->samples : DEFAULT_SDL_AUDIO_SAMPLES;
    FWantedSpec.callback = TSDLAudioDev::AudioCallback;
    FWantedSpec.userdata = this;
    if (FWantedSpec.samples > MAX_SDL_AUDIO_SAMPLES)
        FWantedSpec.samples = MAX_SDL_AUDIO_SAMPLES;

    FDevId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(FDevIdx, FIsRecord),
        FIsRecord, &FWantedSpec, &FObtainedSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (FDevId < 0)
    {
        printf("open audio dev: %s failed: %d \n", 
            TSDLAudioDev::DeviceName(FDevIdx, FIsRecord), FDevId);
        return FDevId;
    }
    FState = AUDIO_STATE_OPEN;
    // Params(params);
    return 0;
}

void TSDLAudioDev::Params(audio_params_t *params)
{
    if (params)
    {
        params->format = TSDLAudioDev::ToAudioFormat(FObtainedSpec.format);
        params->channels = FObtainedSpec.channels;
        params->sample_rate = FObtainedSpec.freq;
        params->samples = FObtainedSpec.samples;
    }
}

void TSDLAudioDev::Close()
{
    SDL_CloseAudioDevice(FDevId);
    FState = AUDIO_STATE_CLOSED;
}

int TSDLAudioDev::Pause(int enable)
{
    SDL_PauseAudioDevice(FDevId, enable > 0 ? SDL_TRUE : SDL_FALSE);
    if (enable)
        FState = AUDIO_STATE_PAUSED;
    else 
        FState = AUDIO_STATE_RUNNING;
    return 0;
}

int TSDLAudioPlay::Open(audio_params_t *params)
{
    int ret;
    if ((ret = inherited::Open(params)) < 0)
        return ret;

    FBuffer = new TLoopBuffer(8 * FObtainedSpec.samples * FObtainedSpec.channels *
        SDL_AUDIO_BITSIZE(FObtainedSpec.format) / 8);

    if (! FBuffer)
        return -ENOMEM;
    return 0;
}

void TSDLAudioPlay::Close()
{
    inherited::Close();
    if (FBuffer)
    {
        delete FBuffer;
        FBuffer = NULL;
    }
}

void TSDLAudioPlay::HandleCallback(uint8_t *stream, int len)
{
    size_t ret;
    if (FAudioSource)
    {
        audio_frame_t frame;
        frame.buf = stream;
        frame.len = len;
        FAudioSource->OnFillFrame(this, &frame);
    }
    else
    {
        if ((ret = FBuffer->Read(stream, len)) != len)
        {
            if (FReadRetried == 0)
                printf("buffer is null, reading:%d readed: %d\n", len, ret);
            if (ret == 0)
            {
                if (FReadRetried == 4)
                {
                    printf("audio play, read timeout!\n");
                    // To do...
                }
                else
                    FReadRetried++;
            }
            FReadRetried %= 20;
            memset(stream + ret, 0, len - ret);
        }
        else 
            FReadRetried = 0;
    }
}

void TSDLAudioPlay::Clear()
{
    if (FBuffer)
        FBuffer->Clear();
}

ssize_t TSDLAudioPlay::Write(unsigned char *buf, size_t count)
{
    size_t ret;
    if (! FBuffer)
        return -ENOMEM;     
    if ((ret = FBuffer->Write(buf, count)) != count)
    {
        // printf("buffer not enough, writing: %d writed: %d\n", count, ret);
    }

    return ret;
}

void TSDLAudioRecord::HandleCallback(uint8_t *stream, int len)
{
    audio_frame_t frame;
    frame.buf = (unsigned char *)stream;
    frame.len = len;

    TAudioSource::Send(&frame);
}