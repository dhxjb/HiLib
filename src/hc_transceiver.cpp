#include "hc_transceiver.h"

#include <stdio.h>
#include <unistd.h>

#include "h264_util.h"

using namespace HiCreation;

int TLVTransceiver::SendTLV(int type, int len, const void *value)
{
    TLV_t sending_tlv;
    sending_tlv.type = type;
    sending_tlv.len = len;
    int sending_len = sizeof(TLV_t) - sizeof(char *);
    int sent;

    if((sent = inherited::Send(&sending_tlv, sending_len)) == sending_len)
    {
        if ((sent = inherited::Send(value, len)) != len)
            printf("failed to send tlv payload \n");
    }
    else
    {
        printf("failed to send tlv header \n");
        sent = 0;
    }

    return sent;
}

int TLVTransceiver::ReceiveTLV(TLV_t *tlv)
{
    int reading_len = sizeof(TLV_t) - sizeof(char *);
    int received;
    int ret = 0;
    if ((received = Receive(tlv, reading_len)) == reading_len)
    {
        // printf("received header: %d %d \n", tlv->Type, tlv->Len);
        ret += received;
        if (tlv->len > 0)
        {
            tlv->value = new char[tlv->len];
            if ((received = Receive(tlv->value, tlv->len)) != tlv->len)
                printf("received payload deficient: %d %d \n", tlv->len, received);
        }
        else
            return 0;
    }
    else
    {
        printf("err received: %d \n", received);
        return 0;
    }
    ret += received;
    return ret;
}

void TLVTransceiver::Release(TLV_t *tlv)
{
    if (tlv && tlv->value)
    {
        delete[] tlv->value;
    }
}

int TH264Transceiver::SendFrame(const void *buf, int count)
{
    if (count < 5)
    {
        printf("not h264 frame buffer \n");
        return 0;
    }

    const uint8_t *h264_buf = (uint8_t *)buf;
    // printf("header: %x %x %x %x %x \n", H264Buf[0], H264Buf[1], H264Buf[2], H264Buf[3], H264Buf[4]);
    int type = TH264Utils::ParseNaluType(h264_buf[sizeof(H264_MARKER)]);

    return inherited::SendTLV(type, count, buf);
}

int TH264Transceiver::NextFrame(TLV_t *frame)
{
    return inherited::ReceiveTLV(frame);
}

void TH264Transceiver::ReleaseFrame(TLV_t *frame)
{
    return inherited::Release(frame);
}
