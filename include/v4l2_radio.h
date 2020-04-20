#ifndef __HC_V4L2_RADIO_H__
#define __HC_V4L2_RADIO_H__

#include <stdint.h>
#include "v4l2_device.h"

#define DEFAULT_RADIO_DEV_FILE  "/dev/radio0"

#define V4L2_CID_PRIVATE_RADIO_POWER   (V4L2_CID_USER_BASE + 0x1000)
#define V4L2_CID_PRIVATE_RADIO_SWITCH  (V4L2_CID_USER_BASE + 0x1001)

namespace HiCreation
{
    typedef struct RADIO_status_t
    {
        uint32_t freq;
        // dBμV
        uint8_t RSSI;
        // dB
        uint8_t SNR;
        // flags
        uint8_t flags;
    } RADIO_status_t;

    enum RADIO_seekdir_t
    {
        SEEK_UP,
        SEEK_DOWN
    };

    class TV4L2RadioCtrl
    {
    public:
        TV4L2RadioCtrl(TV4L2Device *dev):
            FRadioDev(dev), 
            FCurrFreq(0), Fac(16000)
        {}

        /*private operation, when driver not supported, ngnore*/
        int StartUp();
        void ShutDown();
        int Reset();

        uint32_t Freq();
        int QueryStat(RADIO_status_t *status);

        /*freq is kHz*/
        int Tune(uint32_t freq, RADIO_status_t *status);
        int Seek(RADIO_seekdir_t dir, RADIO_status_t *status);

    protected:
        int SWPower(int enable);
        int HWPower(int enable);

    protected:
        TV4L2Device *FRadioDev;
        uint32_t FCurrFreq;
        double Fac;
    };
};

#endif