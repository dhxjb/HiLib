#ifndef __AUDIO_MIXER_H__
#define __AUDIO_MIXER_H__

#include <list>

#include "hc_audio.h"
#include "sdl_audio.h"

#define DEFALUTL_PLAY_DEVICE_NAME    "audiocodec"
#define MAX_MIXER_CHANNELS           3

namespace HiCreation
{
    /*Just fake now, only one channel active, to do...*/
    class TAudioMixer : public TSDLAudioPlay
    {
        typedef TSDLAudioPlay inherited;
    public:
        static TAudioMixer* InstRef();
        static void Release();
        
    public:
        TAudioMixer():
            TSDLAudioPlay(DEFALUTL_PLAY_DEVICE_NAME),
            FRunningSource(NULL)
        {}

        virtual int Open(audio_params_t *params) override;
        // 
        virtual int Add(IAudioSource *source);
        virtual void Remove(IAudioSource *source);

        virtual void Pause(IAudioSource *source, int enable);

        // to do...
        virtual int MixVolume(IAudioSource *source, int volume)
            { return 0; }
 
        virtual void OnFrame(audio_frame_t *frame) override;

    protected:
        virtual int Pause(int enable) override
            { return inherited::Pause(enable); }

    private:
        IAudioSource *FRunningSource;
        std::list<IAudioSource *> FPausedSources;
    };
};


#endif