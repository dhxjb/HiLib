#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "v4l2_radio.h"

using namespace HiCreation;

int TV4L2RadioCtrl::StartUp(RADIO_modulation_t mode)
{
    int ret;
    struct v4l2_tuner vtuner;
    if ((ret = FRadioDev->Open(0)) < 0)
    {
        printf("%s open err: %d\n", FRadioDev->Name(), ret);
        return ret;
    }

    memset(&vtuner, 0, sizeof(struct v4l2_tuner));
    vtuner.index = 0;
    if ((ret = FRadioDev->GetTuner(&vtuner)) != 0)
    {
        printf("get tuner failed: %d\n", ret);
        return ret;
    }

    if (vtuner.capability & V4L2_TUNER_CAP_LOW)
        Fac = 16000;
    else if (vtuner.capability & V4L2_TUNER_CAP_1HZ)
        Fac = 1000000;
    else
        Fac = 16;

    FMode = mode;
    printf("fac: %f \n", Fac);
    return 0;
}

void TV4L2RadioCtrl::ShutDown()
{
    if (! FRadioDev->IsRunning())
        return;
}

uint32_t TV4L2RadioCtrl::Freq()
{
    RADIO_status_t stat;
    if (FCurrFreq > 0)
        return FCurrFreq;

    if (QueryStat(&stat) == 0)
        return stat.freq;
    else
        return 0;
}

int TV4L2RadioCtrl::QueryStat(RADIO_status_t *status)
{
    int ret;
    struct v4l2_frequency vfreq;
    memset(&vfreq, 0, sizeof(struct v4l2_frequency));
    vfreq.type = V4L2_TUNER_RADIO;
    vfreq.tuner = 0;
    if ((ret = FRadioDev->GetFreq(&vfreq)) < 0)
    {
        printf("Get freq err: %d \n", ret);
        return ret;
    }

    if (vfreq.reserved[0] > 0)
        status->freq = vfreq.reserved[0];
    else 
        status->freq = (uint32_t) vfreq.frequency * 1000 / Fac;
    status->RSSI = (uint8_t) (vfreq.reserved[1] & 0xFF);
    status->SNR = (uint8_t) (vfreq.reserved[2] & 0xFF);
    status->flags = (uint8_t) (vfreq.reserved[3] & 0xFF);
    FCurrFreq = status->freq;

    printf("freq: %d rssi: %d snr: %d \n", status->freq, status->RSSI, status->SNR);
    return 0;
}

int TV4L2RadioCtrl::Tune(uint32_t freq, RADIO_status_t *status)
{
    int ret;
    struct v4l2_frequency vfreq;
    memset(&vfreq, 0, sizeof(struct v4l2_frequency));
    vfreq.type = V4L2_TUNER_RADIO;
    vfreq.tuner = 0;
    vfreq.frequency = freq * Fac / 1000; // freq must convert to MHz from KHz
    if ((ret = FRadioDev->SetFreq(&vfreq)) != 0)
    {
        printf("set freq err: %d \n", ret);
        return ret;
    }
    if (status)
        return QueryStat(status);
    else
        return 0;
}

int TV4L2RadioCtrl::Seek(RADIO_seekdir_t dir, RADIO_status_t *status, bool wrap_around)
{
    int ret;
    struct v4l2_hw_freq_seek vseek;
    memset(&vseek, 0, sizeof(struct v4l2_hw_freq_seek));
    vseek.tuner = 0;
    vseek.type = V4L2_TUNER_RADIO;
    vseek.seek_upward = dir == SEEK_UP ? 1 : 0;
    vseek.wrap_around = wrap_around ? 1 : 0;
    if ((ret = FRadioDev->SeekFreq(&vseek)) != 0)
    {
        printf("seek freq  %d \n", ret);
        return ret;
    }
    if (status)
        return QueryStat(status);
    else
        return 0;
}

int TV4L2RadioCtrl::Ctrl(int id, int value)
{
    int ret;
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = id;
    ctrl.value = value;

    if ((ret = FRadioDev->SetCtrl(&ctrl)) != 0)
        printf("radio private switch ctrl err: %d! \n", ret);

    return ret;
}