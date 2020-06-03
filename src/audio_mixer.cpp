#include "audio_mixer.h"

using namespace HiCreation;

static int RefCount = 0;
static TAudioMixer *FAudioMixerInst;

TAudioMixer* TAudioMixer::InstRef()
{
    if (! FAudioMixerInst)
    {
        FAudioMixerInst = new TAudioMixer();
    }
    if (FAudioMixerInst)
    {
        RefCount++;
        return FAudioMixerInst;
    }
    else 
        return NULL;
}

void TAudioMixer::Release()
{
    if (RefCount <= 0)
        return;
    RefCount--;
    if (RefCount == 0)
    {
        delete FAudioMixerInst;
        FAudioMixerInst = NULL;
    }
}

int TAudioMixer::Open(audio_params_t *params)
{
    if (params && (State() == AUDIO_STATE_PAUSED || State() == AUDIO_STATE_RUNNING))
    {
        if (params->channels != Channels() ||
            params->format != Format() ||
            params->sample_rate != SampleRate())
            Close();
        else 
            return 0;
    }
    if (! params)
    {
        audio_params_t default_params;
        default_params.format = SND_PCM_FORMAT_S16_LE;
        default_params.channels = 2;
        default_params.sample_rate = 44100;
        return inherited::Open(&default_params);
    }
    else
        return inherited::Open(params);
}
// 
int TAudioMixer::Add(IAudioSource *source)
{
    if(! source)
        return -EINVAL;
    if (source == FRunningSource)
        return 0;

    if (FRunningSource)
    {
        FRunningSource->RemoveSink(this);
        FPausedSources.push_front(FRunningSource);
        Clear();
    }
    FRunningSource = source;
    FRunningSource->AddOrUpdateSink(this);
}

void TAudioMixer::Remove(IAudioSource *source)
{
    if (! source)
        return;

    source->RemoveSink(this);
    FPausedSources.remove(source);

    if (source == FRunningSource)
    {
        if (! FPausedSources.empty())
        {
            FRunningSource = FPausedSources.front();
            FPausedSources.pop_front();
            FRunningSource->AddOrUpdateSink(this);
        }
        else 
            FRunningSource = NULL;
    }

    if (! FRunningSource && FPausedSources.empty())
        Close();
}

void TAudioMixer::Pause(IAudioSource *source, int enable)
{
    if (! source)
        return;

    if (FRunningSource == source)
    {
        inherited::Pause(enable);
    }
}

void TAudioMixer::OnFrame(audio_frame_t *frame)
{
    if (! frame)
        return;
    int writed = 0;
    while (writed < frame->len)
    {
        writed += inherited::Write(frame->buf + writed, frame->len - writed);
        if (writed == frame->len)
            break;
        else
            usleep(30 * 1000);
    }
}
