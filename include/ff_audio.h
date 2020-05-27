#ifndef __FF_AUDIO_H__
#define __FF_AUDIO_H__

#include "ff_decoder.h"
#include "hc_thread.h"
#include "sdl_audio.h"

namespace HiCreation
{

    /*Need To do struct change from FFDecoder*/

    class FFAudioPlayer : protected TThread
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

        FFAudioPlayer();
        virtual ~FFAudioPlayer();

        int Init();
        int State() { return FState; }

        bool IsRunning() 
            { return FState == RUNNING; }
        bool IsPaused()
            { return FState == PAUSED; }
        bool IsStopped()
            { return FState == STOPPED; }

        int Load(const char *file);
        int Rewind()
            { return 0;}
        int Play();
        int Pause();
        int Stop();

    protected:
        virtual void Execute(void);

    private:
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

        TSDLAudioPlay *FAudioPlay;
        audio_params_t FAudioParams;
    };
};

#endif
