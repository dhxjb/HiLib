#ifndef __HC_LOOPBUFFER_H__
#define __HC_LOOPBUFFER_H__

#include <stdint.h>
#include <pthread.h>
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

            pthread_mutex_init(&FMutex, NULL);
        }

        virtual ~TLoopBuffer()
        {
            if (FBuf)
                delete[] FBuf;
        }

        size_t Capcity() { return FCapcity; }

        size_t Size() 
        { 
            size_t retval;
            pthread_mutex_lock(&FMutex);
            retval = (FWriteIdx - FReadIdx + FCapcity) % FCapcity;
            pthread_mutex_unlock(&FMutex);
            return retval;
        }

        ssize_t Read(unsigned char *buf, size_t count)
        {
            size_t readable_count;
            pthread_mutex_lock(&FMutex);
            readable_count = (FWriteIdx - FReadIdx + FCapcity) % FCapcity;
            if (count > readable_count)
                count = readable_count;
            if (readable_count > 0)
            {
                if (FReadIdx + count > FCapcity)
                {
                    memcpy(buf, FBuf + FReadIdx, FCapcity - FReadIdx);
                    memcpy(buf + FCapcity - FReadIdx, FBuf, FReadIdx + count - FCapcity);
                    FReadIdx = FReadIdx + count - FCapcity;
                }
                else
                {
                    memcpy(buf, FBuf + FReadIdx, count);
                    FReadIdx += count;
                }
                FReadIdx = FReadIdx % FCapcity;
            }
            pthread_mutex_unlock(&FMutex);
            return count;
        }

        ssize_t Write(unsigned char *buf, size_t count)
        {
            size_t writable_count;
            pthread_mutex_lock(&FMutex);
            writable_count = FCapcity - ((FWriteIdx - FReadIdx + FCapcity) % FCapcity);
            if (count > writable_count)
                count = writable_count;
            if (writable_count > 0)
            {
                if (FWriteIdx + count > FCapcity)
                {
                    memcpy(FBuf + FWriteIdx, buf, FCapcity - FWriteIdx);
                    memcpy(FBuf, buf + FCapcity - FWriteIdx, FWriteIdx + count - FCapcity);
                    FWriteIdx = FWriteIdx + count - FCapcity;
                }
                else
                {
                    memcpy(FBuf + FWriteIdx, buf, count);
                    FWriteIdx += count;
                }
                FWriteIdx = FWriteIdx % FCapcity;
            }
            pthread_mutex_unlock(&FMutex);
            return count;
        }

        void Clear()
        {
            pthread_mutex_lock(&FMutex);
            FReadIdx = FWriteIdx = 0;
            pthread_mutex_unlock(&FMutex);
        }

    protected:
        unsigned char *FBuf;
        size_t FCapcity;
        size_t FReadIdx;
        size_t FWriteIdx;
        pthread_mutex_t FMutex; 
    };
};

#endif