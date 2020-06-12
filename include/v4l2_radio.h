#ifndef __HC_V4L2_RADIO_H__
#define __HC_V4L2_RADIO_H__

#include <stdint.h>
#include "v4l2_device.h"

#define DEFAULT_RADIO_DEV_FILE  "/dev/radio0"

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

    enum RADIO_modulation_t
    {
        RADIO_FM,
        RADIO_AM
    };

    enum RADIO_output_t
    {
        ANALOG_OUTPUT,
        DIGITAL_OUTPUT,
    };

    class TV4L2RadioCtrl
    {
    public:
        TV4L2RadioCtrl(TV4L2Device *dev):
            FMode(RADIO_FM),
            FRadioDev(dev), 
            Fac(16000)
        {}

        TV4L2Device *RadioDev() { return FRadioDev; }

        int Modulation() { return FMode; }

        /*private operation, when driver not supported, ignore*/
        virtual int StartUp(RADIO_modulation_t mode);
        virtual void ShutDown();

        uint32_t Freq();
        int QueryStat(RADIO_status_t *status);

        /*freq is kHz*/
        int Tune(uint32_t freq, RADIO_status_t *status);
        int Seek(RADIO_seekdir_t dir, RADIO_status_t *status, bool wrap_around = true);

        int Ctrl(int id, int value);

    protected:
        RADIO_modulation_t FMode;
        TV4L2Device *FRadioDev;
        double Fac;
    };
};

#endif