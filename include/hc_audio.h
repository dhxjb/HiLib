#ifndef __HC_AUDIO_H__
#define __HC_AUDIO_H__

#include <stdint.h>
#include <alsa/asoundlib.h>

#include "hc_observer.h"

namespace HiCreation
{
    class IAudioSink;
    class IAudioSource;

    typedef struct
    {
        snd_pcm_format_t format;
        uint8_t channels;
        uint32_t sample_rate;
    } audio_params_t;

    typedef struct
    {
        unsigned char *buf;
        uint32_t len;
    } audio_frame_t;

    class IAudioSource : protected IObservable
    {
        typedef IObservable inherited;
    public:
        virtual void AddOrUpdateSink(IAudioSink *sink)
            { return inherited::AddObserver((IObserver *)sink); }
        virtual void RemoveSink(IAudioSink *sink)
            { return inherited::DeleteObserver((IObserver *)sink); }
    };

    class IAudioSink : public IObserver
    {
    public:
        virtual void OnFrame(audio_frame_t *frame) = 0;
        virtual void OnDiscardedFrame() {};
    
    protected:
        virtual void Update(void *arg)
        {
            if (arg)
                OnFrame((audio_frame_t *)arg);
            else
                OnDiscardedFrame();
        }
    };

    class IAudioDev
    {
    public:
        virtual int Open(audio_params_t *params, bool isrecord) = 0;
        virtual void Close() = 0;

        virtual int Pause(int enable) = 0;

        virtual const char *Name() = 0;
        virtual snd_pcm_format_t Format() = 0;
        virtual uint8_t Channels() = 0;
        virtual uint32_t SampleRate() = 0;

        virtual uint32_t BufferSize() = 0;

        virtual ssize_t Read(unsigned char *buf, size_t count) = 0;
        virtual ssize_t Write(unsigned char *buf, size_t count) = 0;
    };
};

#endif