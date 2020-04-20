#ifndef __HC_TRANSCEIVER_H__
#define __HC_TRANSCEIVER_H__

#include "uc_stream.h"

using namespace UltraCreation;

namespace HiCreation
{
    typedef struct
    {
        int type;
        int len;
        char *value;
    } TLV_t;

    class Transceiver
    {
    public:
        Transceiver(TStream *stream):
            FStream(stream)
        {
        }

        virtual ~Transceiver() {}

        virtual int Send(const void *buf, int len)
        {
            if (FStream)
                return FStream->WriteBuf(buf, len);
            else
                return 0;
        }

        virtual int Receive(void *buf, int len)
        {
            if (FStream)
                return FStream->ReadBuf(buf, len);
            else
                return 0;
        }

    protected:
        TStream *FStream;
    };

    class TLVTransceiver: protected Transceiver
    {
    private:
        typedef Transceiver inherited;

    public:
        TLVTransceiver(TStream *stream):
            inherited(stream)
        {
        }

        virtual ~TLVTransceiver() {}

        int SendTLV(int type, int len, const void *value);
        int ReceiveTLV(TLV_t *tlv);
        void Release(TLV_t *tlv);
    };

    class TH264Transceiver: public TLVTransceiver
    {
    private:
        typedef TLVTransceiver inherited;

    public:
        TH264Transceiver(THandleStream *stream) :
            inherited(stream)
        {
        }

        virtual ~TH264Transceiver() {}

        int SendFrame(const void *buf, int count);
        int NextFrame(TLV_t *frame);
        void ReleaseFrame(TLV_t *tlv);
    };
};

#endif // __HC_TRANSCEIVER_H
