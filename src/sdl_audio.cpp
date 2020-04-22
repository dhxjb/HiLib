#include <stdio.h>
#include <string.h>

#include "sdl_audio.h"

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
    FIsRecord(isrecord), FDevId(-1)
{
    memset(&FWantedSpec, 0, sizeof(FWantedSpec));
    memset(&FObtainedSpec, 0, sizeof(FObtainedSpec));
    FDevIdx = GetDeviceIdx(dev_name);

    if (TSDLAudioDev::Init() < 0)
        printf("sdl init failed\n");
}

TSDLAudioDev::~TSDLAudioDev()
{
    Close();
}

int TSDLAudioDev::Open(audio_params_t *params)
{
    FWantedSpec.format = TSDLAudioDev::ToSDLAudioFormat(params->format);
    FWantedSpec.channels = params->channels;
    FWantedSpec.freq = params->sample_rate;
    FWantedSpec.samples = 1024;
    FWantedSpec.callback = TSDLAudioDev::AudioCallback;
    FWantedSpec.userdata = this;

    FDevId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(FDevIdx, FIsRecord),
        FIsRecord, &FWantedSpec, &FObtainedSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (FDevId < 0)
    {
        printf("open audio dev: %s failed: %d \n", 
            TSDLAudioDev::DeviceName(FDevIdx, FIsRecord), FDevId);
        return FDevId;
    }

    params->format = TSDLAudioDev::ToAudioFormat(FObtainedSpec.format);
    params->channels = FObtainedSpec.channels;
    params->sample_rate = FObtainedSpec.freq;
    return 0;
}

void TSDLAudioDev::Close()
{
    SDL_CloseAudioDevice(FDevId);
}

int TSDLAudioDev::Pause(int enable)
{
    SDL_PauseAudioDevice(FDevId, enable > 0 ? SDL_TRUE : SDL_FALSE);
    return 0;
}

int TSDLAudioPlay::Open(audio_params_t *params)
{
    int ret;
    if ((ret = inherited::Open(params)) < 0)
        return ret;

    FBuffer = new TLoopBuffer(FObtainedSpec.freq * FObtainedSpec.channels *
        SDL_AUDIO_BITSIZE(FObtainedSpec.format) / 8);

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
    if (! FBuffer)
        return;

    if ((ret = FBuffer->Read(stream, len)) != len)
        printf("buffer is null, reading:%d readed: %d\n", len, ret);
}

ssize_t TSDLAudioPlay::Write(unsigned char *buf, size_t count)
{
    size_t ret;
    if (! FBuffer)
        return -ENOMEM;
        
    if ((ret = FBuffer->Write(buf, count)) != count)
        printf("buffer not enough, writing: %d writed: %d\n", count, ret);

    return ret;
}

void TSDLAudioRecord::HandleCallback(uint8_t *stream, int len)
{
    audio_frame_t frame;
    frame.buf = (unsigned char *)stream;
    frame.len = len;

    IAudioSource::Notify(&frame);
}