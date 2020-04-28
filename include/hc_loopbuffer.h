#ifndef __HC_LOOPBUFFER_H__
#define __HC_LOOPBUFFER_H__

#include <stdint.h>
#include <string.h>

namespace HiCreation
{
    class TLoopBuffer
    {
    public:
        TLoopBuffer(size_t capcity):
            TLoopBuffer(NULL, capcity)
        {}

        TLoopBuffer(unsigned char *buf, size_t size):
            FBuf(buf),
            FCapcity(size), FReadIdx(0), FWrited(0)
        {
            if (! FBuf && size > 0)
                FBuf = new unsigned char[size];
        }

        virtual ~TLoopBuffer()
        {
            if (FBuf)
                delete[] FBuf;
        }

        size_t Capcity() 
            { return FCapcity; }
        size_t Size() 
            { return FWrited; }
        bool IsFull()
            { return FWrited == FCapcity; }
        bool IsEmpty()
            { return FWrited == 0; }

        ssize_t Read(unsigned char *buf, size_t count)
        {
            if (FWrited == 0)
                return 0;

            if (count > FWrited)
                count = FWrited;

            if (FReadIdx + count > FCapcity)
            {
                memcpy(buf, FBuf + FReadIdx, FCapcity - FReadIdx);
                memcpy(buf + FCapcity - FReadIdx, FBuf, FReadIdx + count - FCapcity);
            }
            else
            {
                memcpy(buf, FBuf + FReadIdx, count);
            }
            FReadIdx = (FReadIdx + count) % FCapcity;
            FWrited -= count;
            return count;
        }

        ssize_t Write(unsigned char *buf, size_t count)
        {
            size_t writable_count = FCapcity - FWrited;
            size_t write_idx = (FReadIdx + FWrited) % FCapcity;

            if (writable_count == 0)
                return 0;
            if (count >= writable_count)
                count = writable_count;

            if (write_idx + count > FCapcity)
            {
                memcpy(FBuf + write_idx, buf, FCapcity - write_idx);
                memcpy(FBuf, buf + FCapcity - write_idx, write_idx + count - FCapcity);
            }
            else
            {
                memcpy(FBuf + write_idx, buf, count);
            }
            FWrited += count;
            return count;
        }

        void Clear()
        {
            FReadIdx = FWrited = 0;
        }

    protected:
        unsigned char *FBuf;
        size_t FCapcity;
        size_t FReadIdx;
        size_t FWrited;
    };
};

#endif