#ifndef __HC_AUDIO_RECORDER_H__
#define __HC_AUDIO_RECORDER_H__

#include <string.h>

#include "hc_audio.h"
#include "hc_stream.h"

namespace HiCreation
{
    class TAudioRecorder : public IAudioSink
    {
    public:
        TAudioRecorder(const char *name);

        virtual ~TAudioRecorder()
        {
            if (FStream)
                delete FStream;
        }

        // To do...
        int InitParams(audio_params_t *params) { return 0; }

    protected:
        virtual void OnFrame(audio_frame_t *frame);

    protected:
        TFileStream *FStream;
    };
};

#endif