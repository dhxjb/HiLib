#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "sdl_audio.h"
#include "audio_recorder.h"

#define PLAY_DEVICE_NAME    "audiocodec"
#define RECORD_DEVICE_NAME  "sunxi_si473x"

using namespace HiCreation;

static volatile int KeepRunning = 1;

static void IntHandler(int dummy)
{
    printf("%s dummy: %d\n", __func__, dummy);
    KeepRunning = 0;
}

#if 0
#include "hc_loopbuffer.h"
static TLoopBuffer *LoopBuffer;

static void RecordCallback(void *data, Uint8 *stream, int len)
{
    if (LoopBuffer->Write(stream, len) != len)
        printf("%s %d %u,loop buffer is full\n", __func__, len, LoopBuffer->Size());
}

static void PlayCallback(void *data, Uint8 *stream, int len)
{
    if (LoopBuffer->Read(stream, len) != len)
        printf("%s %d %u, loop buffer is not enough\n", __func__, len, LoopBuffer->Size());
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
    uint32_t buf_size;

    signal(SIGINT, IntHandler);

    if ((ret = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS)) < 0)
    {
        printf("sdl init err: %d \n", ret);
        return ret;
    }

    /*if ((ret = SDL_Init(SDL_INIT_VIDEO)) < 0)
    {
        printf("sdl init video err: %d \n", ret);
    }*/

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
    
    RecordingDevId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(1, SDL_TRUE),
        SDL_TRUE, &WantedRecordSpec, &ActualRecordSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (RecordingDevId == 0)
    {
        printf("open record device failed \n");
        return -1;
    }

    buf_size = ActualRecordSpec.freq * \
        ActualRecordSpec.channels * (SDL_AUDIO_BITSIZE(ActualRecordSpec.format) / 8);
    LoopBuffer = new TLoopBuffer(buf_size);
    printf("Loop buffer size: %u \n", LoopBuffer->Capcity());

    SDL_PauseAudioDevice(RecordingDevId, SDL_FALSE);

    memcpy(&WantedPlaySpec, &ActualRecordSpec, sizeof(WantedPlaySpec));
    WantedPlaySpec.callback = PlayCallback;
    PlayingDevId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, SDL_FALSE),
        SDL_FALSE, &WantedPlaySpec, &ActualPlaySpec, SDL_AUDIO_ALLOW_SAMPLES_CHANGE);

    SDL_PauseAudioDevice(PlayingDevId, SDL_FALSE);

    while(KeepRunning)
    {
        usleep(1000);
    }

    SDL_CloseAudioDevice(RecordingDevId);
    SDL_CloseAudioDevice(PlayingDevId);
    if (LoopBuffer)
        delete LoopBuffer;
}

#else
int main(int argc, char **argv)
{
    int ret;
    TSDLAudioPlay *AudioPlay;
    TSDLAudioRecord *AudioRecord;
    TAudioRecorder *AudioRecorder;
    audio_params_t audio_params;

    signal(SIGINT, IntHandler);

    TSDLAudioDev::Init();

    for (int i = 0; i < TSDLAudioDev::Devices(false); i++)
    {
        printf("play device: %s \n", TSDLAudioDev::DeviceName(i, false));
    }

    for (int i = 0; i < TSDLAudioDev::Devices(true); i++)
    {
        printf("record device: %s \n", TSDLAudioDev::DeviceName(i, true));
    }

    AudioRecord = new TSDLAudioRecord(RECORD_DEVICE_NAME);
    AudioPlay = new TSDLAudioPlay(PLAY_DEVICE_NAME);
    AudioRecorder = new TAudioRecorder("/tmp/radio.raw");

    audio_params.format = SND_PCM_FORMAT_S16_LE;
    audio_params.channels = 2;
    audio_params.sample_rate = 48000;

    if (AudioRecord->Open(&audio_params) < 0)
    {
        printf("open record device failed \n");
        goto END;
    }
    printf("record rate: %d channels: %d format: %d \n", 
        audio_params.sample_rate, audio_params.channels, audio_params.format);
    if (AudioPlay->Open(&audio_params) < 0)
    {
        printf("open play device failed \n");
        goto END;
    }
    printf("play rate: %d channels: %d format: %d \n", 
        audio_params.sample_rate, audio_params.channels, audio_params.format);

    AudioRecord->AddOrUpdateSink(AudioPlay);
    AudioRecord->AddOrUpdateSink(AudioRecorder);

    AudioRecord->Pause(0);
    AudioPlay->Pause(0);

    while(KeepRunning)
    {
        usleep(1000);
    }

END:
    delete AudioRecord;
    delete AudioPlay;
    delete AudioRecorder;
}
#endif