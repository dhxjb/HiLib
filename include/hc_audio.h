#ifndef __HC_AUDIO_H__
#define __HC_AUDIO_H__

#include <list>
#include <stdint.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

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

    typedef enum
    {
        AUDIO_STATE_UNKOWN,
        AUDIO_STATE_OPEN,
        AUDIO_STATE_PREPARED,
        AUDIO_STATE_RUNNING,
        AUDIO_STATE_XRUN,
        AUDIO_STATE_PAUSED,
        AUDIO_STATE_CLOSED
    } audio_state_t;

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

        virtual int State() = 0;
        virtual int Pause(int enable) = 0;

        virtual const char *Name() = 0;
        virtual snd_pcm_format_t Format() = 0;
        virtual uint8_t Channels() = 0;
        virtual uint32_t SampleRate() = 0;

        virtual uint32_t BufferSize() = 0;

        virtual void Clear() {}
        virtual ssize_t Read(unsigned char *buf, size_t count) = 0;
        virtual ssize_t Write(unsigned char *buf, size_t count) = 0;
    };

    class TAudioSource : public IAudioSource
    {
    public:
        TAudioSource()
        {
            pthread_mutex_init(&FLock, NULL);
        }

        virtual ~TAudioSource()
        {
            pthread_mutex_destroy(&FLock);
        }

        virtual int AddOrUpdateSink(IAudioSink *sink)
        {
            pthread_mutex_lock(&FLock);
            FSinks.push_back(sink);
            pthread_mutex_unlock(&FLock);           
            return 0;
        }
            
        virtual void RemoveSink(IAudioSink *sink)
        {
            pthread_mutex_lock(&FLock);
            FSinks.remove(sink);
            pthread_mutex_unlock(&FLock);
        }

        virtual void Send(audio_frame_t *frame)
        {
            if (FSinks.empty())
                return;

            std::list<IAudioSink *>::iterator iter;
            pthread_mutex_lock(&FLock);
            for (iter = FSinks.begin(); iter != FSinks.end(); ++iter)
            {
                if ((*iter))
                    (*iter)->OnFrame(frame);
            }
            pthread_mutex_unlock(&FLock); 
        }

        virtual int OnFillFrame(IAudioSink *sink, audio_frame_t *frame)
            { return -EACCES; }

    protected:
        std::list<IAudioSink *> FSinks;
        pthread_mutex_t FLock;
    };
};

#endif