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

        // Todo...
        int BufferSize() { return -1; }
        int FrameWidth() { return -1; }
        int FrameHeight() { return -1; }

    protected:
        TV4L2Device *FVideoDev;
    };

};

#endif