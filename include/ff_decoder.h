#ifndef __FF_DECODER_H__
#define __FF_DECODER_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

#ifdef __cplusplus
}
#endif // __cplusplus

#include "hc_list.h"
#include "hc_audio.h"

namespace HiCreation
{
    class FFDecoder;

    class IPacketSource
    {
    public:
        
    };

    class IPacketSink
    {
    public:
        
    };

    class TAVFormat
    {
    public:
        int Open();

        FFDecoder* Decoder(AVMediaType type);
        
    protected:
        AVPacket FPkt;
        TList<AVPacket> FAudioQueue;
        TList<AVPacket> FVideoQueue; 
        TList<AVPacket> FSubQueue;
    };

    class FFDecoder
    {
    public:



    protected:
        AVCodecContext *FCodecCtx;
        AVFrame FFrame;
    };

    class FFAudioDecoder : IAudioSource
    {
    public:

    };
};

#endif