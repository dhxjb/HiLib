#ifndef __HC_STREAM_H__
#define __HC_STREAM_H__

#include "uc_stream.h"

#define HI_STREAM_PIPE_NAME "histreampipe"

using namespace UltraCreation;

namespace HiCreation
{
    class TFileStream: public THandleStream
    {
    private:
        typedef THandleStream inherited;

    public:
        TFileStream(int fd):
            inherited(fd)
        {}

        static TFileStream* Create(const char *path, int flags);
    };

    class TPipeStream: public THandleStream
    {
    private:
        typedef THandleStream inherited;

    public:
        enum EndPointType
        {
            WRITE,
            READ
        };

    public:
        using inherited::Read;
        using inherited::ReadBuf;

        using inherited::Write;
        using inherited::WriteBuf;

        static TPipeStream* Create(const char *path, EndPointType type);

        TPipeStream(int fd, EndPointType type):
            inherited(fd), FType(type)
        {}


    protected:
        virtual off_t Position(void)
        {
            return UltraCreation::__set_errno(EACCES);
        }

        virtual ssize_t Seek(off_t offset, int origin)
        {
            return UltraCreation::__set_errno(EACCES);
        }


    protected:
        EndPointType FType;
    };

    class TCPStream: public THandleStream
    {
    private:
        typedef THandleStream inherited;

    public:
        using inherited::Read;
        using inherited::ReadBuf;

        using inherited::Write;
        using inherited::WriteBuf;

        TCPStream(int fd):
            inherited(fd)
        {}


    protected:
        virtual off_t Position(void)
        {
            return UltraCreation::__set_errno(EACCES);
        }

        virtual ssize_t Seek(off_t offset, int origin)
        {
            return UltraCreation::__set_errno(EACCES);
        }
    };
};

#endif // __HC_STREAM_H
