#ifndef __HC_AUDIO_H__
#define __HC_AUDIO_H__

#include <stdint.h>
#include <alsa/asoundlib.h>

#include "hc_list.h"

namespace HiCreation
{
    class IAudioSink;
    class IAudioSource;

    typedef struct
    {
        snd_pcm_format_t format;
        uint8_t channels;
        uint32_t sample_rate;
        uint32_t samples;
    } audio_params_t;

    typedef struct
    {
        unsigned char *buf;
        uint32_t len;
        int samples;
        uint64_t pts;
    } audio_frame_t;

    /*Audio Source ----> Sink*/
    /*interface with two way access, source active pushed or sink active pulled */
    /*implement with either or both*/
    class IAudioSource
    {
    public:
        virtual int AddOrUpdateSink(IAudioSink *sink) = 0;
        virtual void RemoveSink(IAudioSink *sink) = 0;

        /*for sink to pull actively*/
        virtual int OnFillFrame(IAudioSink *sink, audio_frame_t *frame) = 0;
    };

    class IAudioSink
    {
    public:
        /*for source to push actively*/
        virtual void OnFrame(audio_frame_t *frame) = 0;

        virtual void SetSource(IAudioSource *source) {} 
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

    class TAudioSource : public IAudioSource
    {
    public:
        virtual int AddOrUpdateSink(IAudioSink *sink)
        { 
            FSinks.PushBack(sink);
            return 0;
        }
            
        virtual void RemoveSink(IAudioSink *sink)
        {
            TList<IAudioSink *>::Iterator iter = FSinks.Begin();
            while (iter != FSinks.End())
            {
                if ((*iter) == sink)
                {
                    FSinks.Extract(iter);
                    break;
                }
                iter++;
            }
        }

        virtual void Send(audio_frame_t *frame)
        {
            if (FSinks.IsEmpty())
                return;

            TList<IAudioSink *>::Iterator iter;
            for (iter = FSinks.Begin(); iter != FSinks.End(); iter++)
                (*iter)->OnFrame(frame);
        }

        virtual int OnFillFrame(IAudioSink *sink, audio_frame_t *frame)
            { return -EACCES; }

    protected:
        TList<IAudioSink *> FSinks;
    };
};

#endif