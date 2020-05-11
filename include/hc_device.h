#ifndef __HC_DEVICE_H__
#define __HC_DEVICE_H__

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string>

namespace HiCreation
{
    class TDevFile
    {
    public:
        TDevFile(const char *name):
            FName(name)
        {}

        TDevFile(const std::string name):
            FName(name)
        {}

        virtual ~TDevFile()
        {
            Close();
        }

        const char* Name() const { return FName.c_str(); }

        int Fd() { return FHandle; }

        virtual bool IsRunning() { return FHandle > 0; }

        virtual int Open(int flag = O_RDWR)
        { 
            if (FHandle <= 0)
                FHandle = open(FName.c_str(), flag);
            return FHandle;
        }

        virtual void Close()
        {
            if (FHandle) close(FHandle);
            FHandle = -1;
        }

        virtual ssize_t Read(unsigned char *buf, size_t count)
        {
            if (FHandle > 0)
                return read(FHandle, buf, count);
            else
                return -ENODEV;
        }

        virtual ssize_t Write(unsigned char *buf, size_t count)
        {
            if (FHandle > 0)
                return write(FHandle, buf, count);
            else
                return -ENODEV;
        }

        int Ioctl(unsigned long request, void *data)
        {
            if (FHandle > 0)
                return ioctl(FHandle, request, data);
            else
                return -ENODEV;
        }
    
    protected:
        int FHandle;
        std::string FName;
    };
};

#endif