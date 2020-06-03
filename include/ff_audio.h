#ifndef __FF_AUDIO_H__
#define __FF_AUDIO_H__

#include "ff_decoder.h"
#include "hc_thread.h"
#include "sdl_audio.h"
#include "audio_mixer.h"

namespace HiCreation
{
    /*Need To do struct change from FFDecoder*/
    class FFAudioPlayer : public IAudioSource, protected TThread
    {
        typedef enum
        {
            UNKOWN,
            LOADING,
            LOADED,
            RUNNING,
            PAUSED,
            STOPPING,
            STOPPED,
        } state_t;

    public:
        static FFAudioPlayer* InstRef();
        static void Release();
        
    public:
        FFAudioPlayer();
        virtual ~FFAudioPlayer();

        int Init();
        int State() { return FState; }

        bool IsRunning() 
            { return FState == RUNNING; }
        bool IsPaused()
            { return FState == PAUSED; }
        bool IsStopped()
            { return FState != RUNNING && FState != PAUSED; }

        int Load(const char *file);
        int Rewind();
        int Play(uint8_t loop = 1);
        int Pause();
        int Stop();

        virtual int AddOrUpdateSink(IAudioSink *sink) override;
        virtual void RemoveSink(IAudioSink *sink) override;

        /*for sink to pull actively*/
        virtual int OnFillFrame(IAudioSink *sink, audio_frame_t *frame) override
            { return 0; }

    protected:
        virtual void Execute(void);

    private:
        int FLoop;
        state_t FState;
        int FAudioIdx;
        bool FThreadDone;
        AVFormatContext *FFormatCtx;
        AVCodecContext *FCodecCtx;
        struct SwrContext *FSwrCtx;
        uint8_t *FBuf;
        int FBufSize;
        int FBufReaded;
        AVFrame *FFrame; 
        AVPacket FPkt;

        TAudioMixer *FAudioMixer;
        IAudioSink *FAudioSink;
        bool FPausedByRace;
        audio_params_t FAudioParams;
    };
};

#endif
