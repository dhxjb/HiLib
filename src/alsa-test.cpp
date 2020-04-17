#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "alsa_device.h"

#define R11_ALSA_CAPTURE_NAME   "hw:1,0"
#define R11_ALSA_PLAY_NAME      "default"

using namespace HiCreation;

// si473x supported rates
// SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000
// si473x supported formats
// SND_PCM_FORMAT_S8 | SND_PCM_FORMAT_S16_LE | SND_PCM_FORMAT_S24_LE

static volatile int KeepRunning = 1;

static void IntHandler(int dummy)
{
    printf("%s dummy: %d\n", __func__, dummy);
    KeepRunning = 0;
}

int main(int argc, char **argv)
{
    int ret;
    alsa_params_t alsa_params;
    unsigned char *audio_buf;
    unsigned long audio_buf_size;
    TALSADevice *ALSACapture, *ALSAPlay;

    signal(SIGINT, IntHandler);

    ALSACapture = new TALSADevice(R11_ALSA_CAPTURE_NAME);
    if (! ALSACapture)
    {
        printf("create alsa device: %s failed \n", R11_ALSA_CAPTURE_NAME);
        return -ENODEV;
    }
    ALSAPlay = new TALSADevice(R11_ALSA_PLAY_NAME);
    if (! ALSAPlay)
    {
        printf("create alsa device: %s failed \n", R11_ALSA_PLAY_NAME);
        return -ENODEV;
    }

    alsa_params.format = SND_PCM_FORMAT_S16_LE;
    alsa_params.sample_rate = 48000;
    alsa_params.channels = 2;

    if (ALSACapture->Open(&alsa_params, true) < 0)
    {
        printf("open alsa: %s failed\n", R11_ALSA_CAPTURE_NAME);
        return -ENODEV;
    }

    if (ALSAPlay->Open(&alsa_params, false) < 0)
    {
        printf("open alsa: %s play failed\n", R11_ALSA_PLAY_NAME);
        delete ALSACapture;
        return -ENODEV;
    }

    if ((audio_buf_size = ALSACapture->BufferSize()) > 0)
    {
        audio_buf = new unsigned char[audio_buf_size];
    }
    else 
        goto END;

    ALSAPlay->Pause(false);
    while(KeepRunning)
    {
        ret = ALSACapture->Read(audio_buf, audio_buf_size);
        printf("readed: %d \n", ret);
        if (ret > 0)
            ret = ALSAPlay->Write(audio_buf, ret);
        printf("writed: %d \n", ret);
    }

    ALSAPlay->Pause(true);

END:
    delete ALSAPlay;
    delete ALSACapture;
    delete[] audio_buf;

    return 0;
}