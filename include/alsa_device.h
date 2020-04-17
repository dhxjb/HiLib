#ifndef __ALSA_DEVICE_H__
#define __ALSA_DEVICE_H__

#include <alsa/asoundlib.h>

/**
 *          +----APP----+
 *                |
 *          +--ALSA LIB-+  
 * user space     |
 * ---------------|------------------
 * kernel space   v
 *         +----------------+
 *         |   ALSA Core    |
 *         |PCM|CONTROL|MIDI|
 *         +----------------+
 *                 |
 *         +---ASoC CORE----+
 *                 |
 *       +----------------------+
 *       |      HW driver       |
 *       |Machine|Platform|Codec|
 *       +----------------------+
 * 
 *    buffer = peroid_count * period
 *    period = frame_count * frame
 *    frame = channels(Mono is 1, Stereo is 2) * sample 
 *    sample = 8bit | 16bit | 24bit | 32bit
 *    bps = channels * sample * sample_rate
 * */


namespace HiCreation
{
    #define DEFAULT_ALSA_DEVICE "default"
    #define ALSA_DEBUG_OUTPUT

    typedef struct alsa_params_t
    {
        snd_pcm_format_t format;
        unsigned int channels;
        unsigned int sample_rate;
    } alsa_params_t;

    class TALSADevice
    {
    public:
        TALSADevice(const char *device_name);
        virtual ~TALSADevice();

        /*True is capture, False is play*/
        int Open(alsa_params_t *params, bool iscapture);
        void Close();

        bool IsRunning() { return State() == SND_PCM_STATE_RUNNING; }
        int Pause(bool enable);

        int Fd();
        snd_pcm_t *PcmHandle() { return FPcmHandle; }
        snd_pcm_uframes_t BufferSize() { return FBufferSize; }
        snd_pcm_uframes_t PeriodSize() { return FPeriodSize; }
        ssize_t Read(unsigned char *buf, size_t count);
        ssize_t Write(unsigned char *buf, size_t count);

        int SetDataSilence(unsigned char *buf, size_t count)
            { return snd_pcm_format_set_silence(FParams.format, buf, count); }

        snd_pcm_state_t State();

        snd_pcm_format_t Format() { return FParams.format; }
        int SampleRate() { return FParams.sample_rate; }
        int Channels() { return FParams.channels; }
 
    protected:
        int SetParams(alsa_params_t *params);
        void ShowAvailableFormats(snd_pcm_hw_params_t *params);

    protected:
        char *FPcmName;
        int FCanPause;
        snd_pcm_t *FPcmHandle;
        snd_pcm_stream_t FStream;
        alsa_params_t FParams;
        snd_pcm_state_t FLastState;
        snd_pcm_uframes_t FBufferSize;
        snd_pcm_uframes_t FPeriodSize; 
    #ifdef ALSA_DEBUG_OUTPUT
        snd_output_t *FLog;
    #endif
    };
};

#endif