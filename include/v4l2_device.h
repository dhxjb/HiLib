#ifndef __HC_V4L2_DEVICE_H__
#define __HC_V4L2_DEVICE_H__

#include <string>
#include <linux/videodev2.h>

#include "hc_device.h"

#define MAX_FMT_SIZE 20

namespace HiCreation
{
    class TV4L2Device : public TDevFile
    {
        typedef TDevFile inherited;
    public:
        static TV4L2Device *RadioDev(uint8_t radio_no)
            { return new TV4L2Device(std::string("/dev/radio").append(std::to_string(radio_no))); }
        static TV4L2Device *VideoDev(uint8_t video_no)
            { return new TV4L2Device(std::string("/dev/video").append(std::to_string(video_no))); }
    public:
        TV4L2Device(const char *name):
            inherited(name)
        {}
        TV4L2Device(const std::string name):
            inherited(name)
        {}

        int QueryCap(struct v4l2_capability *cap)
            { return inherited::Ioctl(VIDIOC_QUERYCAP, cap); }

        int MaxFmtSize() { return MAX_FMT_SIZE; } // To do
        int EnumFmt(int idx, struct v4l2_fmtdesc *fmtdesc)
        {   
            fmtdesc->index = idx;
            return inherited::Ioctl(VIDIOC_ENUM_FMT, fmtdesc); 
        }
        int GetFmt(struct v4l2_format *fmt)
            { return inherited::Ioctl(VIDIOC_G_FMT, fmt); }
        int SetFmt(struct v4l2_format *fmt)
            { return inherited::Ioctl(VIDIOC_S_FMT, fmt);}
        int TryFmt(struct v4l2_format *fmt)
            { return inherited::Ioctl(VIDIOC_TRY_FMT, fmt); }

        int QueryCtl(struct v4l2_queryctrl *qctrl)
        {
            qctrl->id |= V4L2_CTRL_FLAG_NEXT_CTRL;
            return inherited::Ioctl(VIDIOC_QUERYCTRL, qctrl);
        }
        int SetCtrl(struct v4l2_control *ctrl)
            { return inherited::Ioctl(VIDIOC_S_CTRL, ctrl); }
        int GetCtrl(struct v4l2_control *ctrl)
            { return inherited::Ioctl(VIDIOC_G_CTRL, ctrl); }

        int GetParam(struct v4l2_streamparm *vparm)
            { return inherited::Ioctl(VIDIOC_G_PARM, vparm); }
        int SetParam(struct v4l2_streamparm *vparm)
            { return inherited::Ioctl(VIDIOC_S_PARM, vparm); }

        int EnumInput(int idx, struct v4l2_input *input)
        {
            input->index = idx;
            return inherited::Ioctl(VIDIOC_ENUMINPUT, input);
        }
        int GetInput(struct v4l2_input *input)
            { return inherited::Ioctl(VIDIOC_G_INPUT, input); }
        int SetInput(struct v4l2_input *input)
            { return inherited::Ioctl(VIDIOC_S_INPUT, input); }

        int QueryBuf(int idx, struct v4l2_buffer *vbuf)
        {
            vbuf->index = idx;
            return inherited::Ioctl(VIDIOC_QUERYBUF, vbuf); 
        }
        int ReqBufs(struct v4l2_buffer *vbuf)
            { return inherited::Ioctl(VIDIOC_REQBUFS, vbuf); }
        int QBuf(struct v4l2_buffer *vbuf)
            { return inherited::Ioctl(VIDIOC_QBUF, vbuf); }
        int DQBuf(struct v4l2_buffer *vbuf)
            { return inherited::Ioctl(VIDIOC_DQBUF, vbuf); }

        int StreamOn(enum v4l2_buf_type type)
            { return inherited::Ioctl(VIDIOC_STREAMON, &type); }
        int StreamOff(enum v4l2_buf_type type)
            { return inherited::Ioctl(VIDIOC_STREAMOFF, &type); }

        int GetTuner(struct v4l2_tuner *tuner)
            { return inherited::Ioctl(VIDIOC_G_TUNER, tuner); }
        int SetTuner(struct v4l2_tuner *tuner)
            { return inherited::Ioctl(VIDIOC_S_TUNER, tuner); }
        int GetFreq(struct v4l2_frequency *freq)
            { return inherited::Ioctl(VIDIOC_G_FREQUENCY, freq); }
        int SetFreq(struct v4l2_frequency *freq)
            { return inherited::Ioctl(VIDIOC_S_FREQUENCY, freq); }
        int SeekFreq(struct v4l2_hw_freq_seek *seek)
            { return inherited::Ioctl(VIDIOC_S_HW_FREQ_SEEK, seek); }
    };
};

#endif