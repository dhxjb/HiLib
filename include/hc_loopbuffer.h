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
            FCapcity(size), FReadIdx(0), FWriteIdx(0)
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
            { return (FWriteIdx - FReadIdx + FCapcity) % FCapcity; }
        bool IsFull()
            { return (FWriteIdx + 1) % FCapcity == FReadIdx; }
        bool IsEmpty()
            { return FWriteIdx == FReadIdx; }

        ssize_t Read(unsigned char *buf, size_t count)
        {
            size_t readable_count;
            if (FWriteIdx == FReadIdx)
                return 0;

            readable_count = (FWriteIdx + FCapcity - FReadIdx) % FCapcity;
            if (count > readable_count)
                count = readable_count;

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
            return count;
        }

        ssize_t Write(unsigned char *buf, size_t count)
        {
            size_t writable_count;
            writable_count = FCapcity - ((FWriteIdx - FReadIdx + FCapcity) % FCapcity);
            if (writable_count == 0)
                return 0;
            else if (count >= writable_count)
                count = writable_count - 1;

            if (FWriteIdx + count > FCapcity)
            {
                memcpy(FBuf + FWriteIdx, buf, FCapcity - FWriteIdx);
                memcpy(FBuf, buf + FCapcity - FWriteIdx, FWriteIdx + count - FCapcity);
            }
            else
            {
                memcpy(FBuf + FWriteIdx, buf, count);
            }
            FWriteIdx = (FWriteIdx + count) % FCapcity;
            return count;
        }

        void Clear()
        {
            FReadIdx = FWriteIdx = 0;
        }

    protected:
        unsigned char *FBuf;
        size_t FCapcity;
        size_t FReadIdx;
        size_t FWriteIdx;
    };
};

#endif