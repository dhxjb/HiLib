#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "alsa_device.h"

using namespace HiCreation;

TALSADevice::TALSADevice(const char *name):
    FPcmName(NULL), FCanPause(0), FPcmHandle(NULL),
    FLastState(SND_PCM_STATE_DISCONNECTED),
    FBufferSize(0), FPeriodSize(0)
{
    int len = strlen(name);
    if (len == 0)
    {
        printf("%s name is null, use device %s \n", 
            __func__, DEFAULT_ALSA_DEVICE);
    }
    else
    {
        FPcmName = new char[len + 1];
        memcpy(FPcmName, name, len);
        FPcmName[len] = '\0';
    }
}

TALSADevice::~TALSADevice()
{
    Close();
    if (FPcmName)
        delete[] FPcmName;
}

int TALSADevice::Open(alsa_params_t *params, bool is_capture)
{
    int ret;
    if (is_capture)
        FStream = SND_PCM_STREAM_CAPTURE;
    else
        FStream = SND_PCM_STREAM_PLAYBACK;

#ifdef ALSA_DEBUG_OUTPUT
    ret = snd_output_stdio_attach(&FLog, stderr, 0);
    if (ret < 0)
        printf("snd_output_sdio_attach err: %s \n", snd_strerror(ret));
#endif

    ret = snd_pcm_open(&FPcmHandle, FPcmName ? FPcmName : DEFAULT_ALSA_DEVICE,
            FStream, 0);
    if (ret < 0)
    {
        printf("can't open %s , err: %s \n", 
            FPcmName ? FPcmName : DEFAULT_ALSA_DEVICE, snd_strerror(ret));
        return ret;
    }

    memcpy(&FParams, params, sizeof(alsa_params_t));
    ret = SetParams(&FParams);
    if (ret < 0)
    {
        printf("set params err: %d \n", ret);
        goto ERR_PARAMS;
    }
    
    ret = snd_pcm_get_params(FPcmHandle, &FBufferSize, &FPeriodSize);
    if (ret < 0)
    {
        printf("get params err: %s\n", snd_strerror(ret));
        goto ERR_PARAMS;
    }

#ifdef ALSA_DEBUG_OUTPUT
    printf("buffer size: %lu period size: %lu \n", FBufferSize, FPeriodSize);
#endif

    ret = snd_pcm_prepare(FPcmHandle);
    if (ret < 0)
    {
        printf("snd_pcm_prepare err: %s \n", snd_strerror(ret));
        goto ERR_PARAMS;
    }

    printf("%s success!\n", __func__);
    return 0;

ERR_PARAMS:
    Close();
    return ret;
}

void TALSADevice::Close()
{
#ifdef ALSA_DEBUG_OUTPUT
    if (FLog)
    {
        snd_output_close(FLog);
        FLog = NULL;
    }
#endif
    if (FPcmHandle)
    {
        snd_pcm_close(FPcmHandle);
        FPcmHandle = NULL;
    }
}

int TALSADevice::Pause(bool enable)
{
    int ret;
    if (! FPcmHandle)
    {
        printf("pcm is not open!\n");
        return -ENODEV;
    }
    if (! FCanPause)
    {
        printf("pcm can't pause!");
        return 0;
    }
    if ((ret = snd_pcm_pause(FPcmHandle, enable ? 1 : 0)) < 0)
        printf("snd_pcm_pause err: %s %d\n", snd_strerror(ret), ret);
    return 0; // ignore pause error
}

int TALSADevice::Fd() 
{
    int ret;
    int count;
    struct pollfd pfds[1];
    if (! FPcmHandle)
        return -ENODEV;

    count = snd_pcm_poll_descriptors_count(FPcmHandle);
    ret = snd_pcm_poll_descriptors(FPcmHandle, pfds, count);
    if (ret < 0)
    {
        printf("snd_pcm_poll_descriptors err: %s \n", snd_strerror(ret));
        return ret;
    }
    return pfds[0].fd;
}

ssize_t TALSADevice::Read(unsigned char *buf, size_t count)
{
    int ret;
    if (count < FPeriodSize)
        return -EINVAL;

    count = FPeriodSize;

    ret = snd_pcm_readi(FPcmHandle, buf, count);
    if (ret < 0)
    {
        if (ret == -EAGAIN)
        {
            printf("again err, wait 100ms\n");
            snd_pcm_wait(FPcmHandle, 100);
        }
        else if (ret == -EPIPE)
        {
            printf("pipe err, restart pcm\n");
            snd_pcm_prepare(FPcmHandle);
        }
        else 
            printf("pcm read err: %s\n", snd_strerror(ret));
        return ret;
    }
    return count;
}

ssize_t TALSADevice::Write(unsigned char *buf, size_t count)
{
    int ret;
    if (count < FPeriodSize)
        return -EINVAL;

    ret = snd_pcm_writei(FPcmHandle, buf, count);
    if (ret < 0)
    {
        if (ret == -EAGAIN)
        {
            printf("again err, wait 100ms\n");
            snd_pcm_wait(FPcmHandle, 100);
        }
        else if (ret == -EPIPE)
        {
            printf("pipe err, restart pcm\n");
            snd_pcm_prepare(FPcmHandle);
        }
        else 
            printf("pcm write err: %s\n", snd_strerror(ret));
        return ret;
    }
    return count;
}

snd_pcm_state_t TALSADevice::State()
{
    snd_pcm_status_t *status;
    int ret;

    snd_pcm_status_alloca(&status);
    if ((ret = snd_pcm_status(FPcmHandle, status)) < 0)
    {
        printf("snd_pcm_status err: %s \n", snd_strerror(ret));
        return FLastState;
    }

    return snd_pcm_status_get_state(status);
}

int TALSADevice::SetParams(alsa_params_t *params)
{
    int ret;
    snd_pcm_hw_params_t *hw_params;

    snd_pcm_hw_params_alloca(&hw_params);
    if ((ret = snd_pcm_hw_params_any(FPcmHandle, hw_params)) < 0)
    {
        printf("snd_pcm_hw_params_any err: %s \n", snd_strerror(ret));
        return ret;
    }

#ifdef ALSA_DEBUG_OUTPUT
    fprintf(stderr, "HW Params of device: %s \n", snd_pcm_name(FPcmHandle));
    fprintf(stderr, "------------------\n");
    snd_pcm_hw_params_dump(hw_params, FLog);
    fprintf(stderr, "------------------\n");
#endif

    if ((ret = snd_pcm_hw_params_set_access(FPcmHandle, hw_params,
            SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        printf("snd_pcm_hw_params_set_access err: %s \n", snd_strerror(ret));
        return ret;
    }

    if ((ret = snd_pcm_hw_params_set_format(FPcmHandle, hw_params, FParams.format)) < 0)
    {
        printf("Set format err: %s \n", snd_strerror(ret));
        ShowAvailableFormats(hw_params);
        return ret;
    }

    if ((ret = snd_pcm_hw_params_set_channels(FPcmHandle, hw_params, FParams.channels)) < 0)
    {
        printf("Set channels err: %s \n", snd_strerror(ret));
        return ret;
    }

    if ((ret = snd_pcm_hw_params_set_rate_near(FPcmHandle, hw_params, 
            &FParams.sample_rate, 0)) < 0)
    {
        printf("Set sample rate err: %s \n", snd_strerror(ret));
        return ret;
    }

    FCanPause = snd_pcm_hw_params_can_pause(hw_params);
    printf("can pause: %d \n", FCanPause);

    if ((ret = snd_pcm_hw_params(FPcmHandle, hw_params)) < 0)
    {
        printf("snd_pcm_hw_params err: %s \n", snd_strerror(ret));
        return ret;
    }

    return 0;   
}

void TALSADevice::ShowAvailableFormats(snd_pcm_hw_params_t *params)
{
    int format;
    printf("Available formats: \r\n");
    for (format = 0; format <= SND_PCM_FORMAT_LAST; format++)
    {
        if (snd_pcm_hw_params_test_format(FPcmHandle, params, (snd_pcm_format_t)format) == 0)
            printf("- %s\n", snd_pcm_format_name((snd_pcm_format_t)format));  
    }
}
