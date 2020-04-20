#ifndef __HC_V4L2_VIDEO_H__
#define __HC_V4L2_VIDEO_H__

#include "v4l2_device.h"

namespace HiCreation
{
    class TV4L2VideoCapturer
    {
    public:
        TV4L2VideoCapturer(TV4L2Device *dev):
            FVideoDev(NULL)
        {}

        virtual ~TV4L2VideoCapturer()
        {}

    protected:
        TV4L2Device *FVideoDev;
    };

};

#endif