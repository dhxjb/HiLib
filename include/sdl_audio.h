#ifndef __SDL_AUDIO_H__
#define __SDL_AUDIO_H__

#include <stdint.h>
#include <errno.h>
#include "SDL.h"

#include "hc_loopbuffer.h"
#include "hc_audio.h"

namespace HiCreation
{
    class TSDLAudioDev : public IAudioDev
    {
    public:
        static int Init(uint32_t flags = 0)
            { return SDL_Init(SDL_INIT_AUDIO | flags); }
        static int Devices(bool isrecord)
            { return SDL_GetNumAudioDevices(isrecord ? SDL_TRUE : SDL_FALSE); }
        static const char* DeviceName(int idx, bool isrecord)
            { return SDL_GetAudioDeviceName(idx, isrecord ? SDL_TRUE : SDL_FALSE); }

        static SDL_AudioFormat ToSDLAudioFormat(snd_pcm_format_t format);
        static snd_pcm_format_t ToAudioFormat(SDL_AudioFormat format);

    public:
        TSDLAudioDev(const char *dev_name, bool isrecord);
        virtual ~TSDLAudioDev();

        const char *Name() { return TSDLAudioDev::DeviceName(FDevIdx, FIsRecord); }

        virtual int Open(audio_params_t *params);
        virtual void Close();
        virtual int Pause(int enable);

        snd_pcm_format_t Format() 
            { return TSDLAudioDev::ToAudioFormat(FObtainedSpec.format); }
        uint32_t SampleRate() 
            { return FObtainedSpec.freq; }
        uint8_t Channels()
            { return FObtainedSpec.channels; }

    protected:
        int Open(audio_params_t *params, bool isrecord)
            { return Open(params); }

        static inline void AudioCallback(void *data, uint8_t *stream, int len)
        {
            TSDLAudioDev *SDLAudioDev = (TSDLAudioDev *)data;
            if (SDLAudioDev)
                SDLAudioDev->HandleCallback(stream, len);
        }
        virtual void HandleCallback(uint8_t *stream, int len) = 0;

        int GetDeviceIdx(const char *dev_name);

    protected:
        bool FIsRecord;
        int FDevIdx;
        int FDevId;
        SDL_AudioSpec FWantedSpec;
        SDL_AudioSpec FObtainedSpec;
    };

    class TSDLAudioPlay : public TSDLAudioDev, public IAudioSink
    {
        typedef TSDLAudioDev inherited;
    public:
        TSDLAudioPlay(const char *name):
            TSDLAudioDev(name, false), FBuffer(NULL), FAudioSource(NULL)
        {}

        uint32_t BufferSize() 
        { 
            if(FBuffer)
                return FBuffer->Capcity();
            else
                return 0;
        }

        virtual int Open(audio_params_t *params);
        virtual void Close();

        ssize_t Write(unsigned char *buf, size_t count);

        virtual void OnFrame(audio_frame_t *frame)
            { Write(frame->buf, frame->len); }

        virtual void SetSource(IAudioSource *source)
            { FAudioSource = source; }
    protected:
        void HandleCallback(uint8_t *stream, int len);
        ssize_t Read(unsigned char *buf, size_t count) { return -EACCES; }

    private:
        TLoopBuffer *FBuffer;
        IAudioSource *FAudioSource;
        int FReadRetried;
    };

    class TSDLAudioRecord : public TSDLAudioDev, public TAudioSource
    {
        typedef TSDLAudioDev inherited;
    public:
        TSDLAudioRecord(const char *name):
            TSDLAudioDev(name, true)
        {}

        /*No buffer, directly post the data to sink*/
        uint32_t BufferSize() { return 0; }
         // to do...
        ssize_t Read(unsigned char *buf, size_t count) { return -ENOSYS; }

    protected:
        void HandleCallback(uint8_t *stream, int len);
        ssize_t Write(unsigned char *buf, size_t count) { return -EACCES; }
    };
};

#endif