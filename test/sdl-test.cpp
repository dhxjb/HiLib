#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "hc_loopbuffer.h"

using namespace HiCreation;

static volatile int KeepRunning = 1;

static void IntHandler(int dummy)
{
    printf("%s dummy: %d\n", __func__, dummy);
    KeepRunning = 0;
}

static void RecordCallback(void *data, Uint8 *stream, int len)
{

}

static void PlayCallback(void *data, Uint8 *stream, int len)
{

}

int main(int argc, char **argv)
{
    int ret;
    SDL_AudioDeviceID RecordingDevId;
    SDL_AudioDeviceID PlayingDevId;
    SDL_Event event;
    SDL_AudioSpec WantedRecordSpec;
    SDL_AudioSpec ActualRecordSpec;
    SDL_AudioSpec WantedPlaySpec;
    SDL_AudioSpec ActualPlaySpec;
    TLoopBuffer *LoopBuffer;
    uint32_t buf_size;

    signal(SIGINT, IntHandler);

    if ((ret = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS)) < 0)
    {
        printf("sdl init err: %d \n", ret);
        return ret;
    }

    if ((ret = SDL_Init(SDL_INIT_VIDEO)) < 0)
    {
        printf("sdl init video err: %d \n", ret);
    }

    for (int i = 0; i < SDL_GetNumAudioDevices(SDL_TRUE); i++)
    {
        printf("record device: %s \n", SDL_GetAudioDeviceName(i, SDL_TRUE));
    }

    WantedRecordSpec.freq = 48000;
    WantedRecordSpec.format = AUDIO_S16SYS;
    WantedRecordSpec.channels = 2;
    WantedRecordSpec.silence = 0;
    WantedRecordSpec.samples = 1024;
    WantedRecordSpec.callback = RecordCallback;
    
    RecordingDevId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, SDL_TRUE),
        SDL_TRUE, &WantedRecordSpec, &ActualRecordSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (RecordingDevId == 0)
    {
        printf("open record device failed \n");
        return -1;
    }

    

    memcpy(&WantedPlaySpec, &ActualRecordSpec, sizeof(WantedPlaySpec));
    WantedPlaySpec.callback = PlayCallback;
    PlayingDevId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, SDL_FALSE),
        SDL_FALSE, &WantedPlaySpec, &ActualPlaySpec, SDL_AUDIO_ALLOW_SAMPLES_CHANGE);

    while(KeepRunning)
    {
        usleep(1000);
    }

    if (LoopBuffer)
        delete LoopBuffer;
}