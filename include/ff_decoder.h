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

#include "hc_audio.h"
#include "hc_list.h"

namespace HiCreation
{
    class IAVPacketSink;
    class FFDecoder;

    class IAVPacketSource
    {
    public:
        virtual int Read(IAVPacketSink *sink) = 0;
    };

    class TAVPacketSink
    {
    public:
        virtual int StreamId() = 0;
        virtual void SetSource(IAVPacketSource *source)
            { FSource = source; }

    protected:
        IAVPacketSource *FSource;
    };

    class TAVFormat
    {
    public:
        TAVFormat()
            {}

        virtual ~TAVFormat()
            { UnInit(); }

        int Init();
        void UnInit();
        int Open(const char *file);
        int Close();

        int ReadFrame(int stream, AVPacket *pkt);

        FFDecoder* Decoder(AVMediaType type);
        
    protected:
        AVFormatContext *FFormatCtx;
        TList<AVPacket> FAudioQueue;
        TList<AVPacket> FVideoQueue; 
        TList<AVPacket> FSubQueue;
    };

    class FFDecoder
    {
    public:
        FFDecoder(TAVFormat *avformat, AVCodecContext *codec_ctx, int stream_idx):
            FAVFormat(avformat), FCodecCtx(codec_ctx), FStreamIdx(stream_idx)
            {}

        int StreamIdx() { return FStreamIdx; }

        virtual int Init();
        virtual int DecodeFrame()
            { return 0; }

    protected:
        TAVFormat *FAVFormat;
        AVCodecContext *FCodecCtx;
        int FStreamIdx;
        AVFrame FFrame;
        AVPacket FPkt;
    };

    class FFAudioDecoder : public FFDecoder, public IAudioSource
    {
        typedef FFAudioDecoder inherited;
    public:
        FFAudioDecoder(TAVFormat *avformat, AVCodecContext *codec_ctx, int stream_idx):
            FFDecoder(avformat, codec_ctx, stream_idx), 
            FSwrCtx(NULL), FSwrBuf(NULL), FBufSize(0), FBufReaded(0)
            {}

        virtual int Init();

        int SetAudioParams(audio_params_t *param);
        audio_params_t *AudioParams()
            { return &FAudioParams; }

        virtual int DecodeFrame();

        virtual int AddOrUpdateSink(IAudioSink *sink)
            {}
        virtual void RemoveSink(IAudioSink *sink)
            {}

        virtual int OnFillFrame(IAudioSink *sink, audio_frame_t *frame);

    protected:
        audio_params_t FAudioParams;
        struct SwrContext *FSwrCtx;
        uint8_t *FSwrBuf;
        int FBufSize;
        int FBufReaded;
    };
};

#endif