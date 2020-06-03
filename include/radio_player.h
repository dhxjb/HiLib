#ifndef __RADIO_PLAYER_H__
#define __RADIO_PLAYER_H__

#include "sdl_audio.h"
#include "audio_mixer.h"

namespace HiCreation
{
    class TRadioPlayer
    {
    public:
        TRadioPlayer(TSDLAudioRecord *radio_dev):
            FRadioDev(radio_dev),
            FAudioMixer(TAudioMixer::InstRef())
        {}

        ~TRadioPlayer()
        {
            Close();
            TAudioMixer::Release();
        }

        int Open(audio_params_t *audio_params)
        {
            int ret;
            ret = FAudioMixer->Open(audio_params);
            if (ret < 0)
                return ret;
            
            FAudioMixer->Params(audio_params);
            return FRadioDev->Open(audio_params);
        }

        void Close()
        {
            if (! FRadioDev)
                return;

            FRadioDev->Close();
            FAudioMixer->Remove(FRadioDev);
        }

        int Play()
        {
            if (! FRadioDev)
                return -ENODEV;

            FAudioMixer->Add(FRadioDev);
            FRadioDev->Pause(0);
            FAudioMixer->Pause(FRadioDev, 0); 
        }

        int Pause()
        {
            if (! FRadioDev)
                return -ENODEV;

            FRadioDev->Pause(1);
            FAudioMixer->Pause(FRadioDev, 1); 
        }

        // to do...
        int SetAudioParams(audio_params_t *params)
            { return 0; }

    private:
        TSDLAudioRecord *FRadioDev;
        TAudioMixer *FAudioMixer;
    };
};

#endif