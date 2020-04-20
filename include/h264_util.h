#ifndef __H264_UTIL_H__
#define __H264_UTIL_H__

#include <stdint.h>

namespace HiCreation
{
    const uint8_t H264_MARKER[] = {0x0, 0x0, 0x0, 0x01};

    class TH264Utils
    {
    public:
        enum NaluType
        {
          kSlice = 1,
          kIdr = 5,
          kSei = 6,
          kSps = 7,
          kPps = 8,
          kAud = 9,
          kEndOfSequence = 10,
          kEndOfStream = 11,
          kFiller = 12,
          kStapA = 24,
          kFuA = 28
        };

        static NaluType ParseNaluType(uint8_t data)
            { return static_cast<NaluType>(data & 0x1F); }
    };
};

#endif // __HC_H264_H
