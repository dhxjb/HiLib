
#ifndef __UC_STREAM_H
#define __UC_STREAM_H

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>

namespace UltraCreation
{
    #define LINEBREAK   "\r\n"

    static inline int __set_errno(int err)
    {
        errno = err;
        return -1;
    }

    class TBaseStream
    {
    public:
        virtual ~TBaseStream()
        {
        }

        virtual ssize_t Seek(off_t offset, int origin)
        {
            return 0;
        }

        virtual off_t Position(void)
        {
            return 0;
        }

        virtual ssize_t Size(void)
        {
            ssize_t pos = Seek(0, SEEK_CUR);
            ssize_t retval = Seek(0, SEEK_END);

            Seek(pos, SEEK_SET);
            return retval;
        }

    protected:
        __attribute__((nonnull))
        virtual ssize_t Read(void *buf, size_t count, uint32_t timeout = 0) = 0;

        __attribute__((nonnull))
        ssize_t ReadBuf(void *buf, size_t count)
        {
            ssize_t readed = Read(buf, count);

            if (readed > 0)
            {
                while (readed < count)
                {
                    ssize_t reading = Read((uint8_t *)buf + readed, count - readed);
                    if (reading <= 0)
                        break;
                    else
                        readed += reading;
                }
            }
            return readed;
        }

        __attribute__((__nonnull__))
        ssize_t ReadLn(char *buf, int bufsize, uint32_t timeout = 0)
        {
            int readed = 0;

            while (readed < bufsize)
            {
                char CH;

                if (Read(&CH, sizeof(CH), timeout) < 0)
                {
                    if (errno == EAGAIN)
                    {
                        continue;
                    }
                    else
                        return -1;
                }

                if (CH == '\n')
                {
                    if (buf[readed -1] == '\r')
                        readed --;

                    buf[readed] = '\0';
                    return readed;
                }
                else
                {
                    buf[readed] = CH;
                    readed ++;
                }
            }

            return __set_errno(EMSGSIZE);
        }

        __attribute__((nonnull))
        virtual ssize_t Write(const void *buf, size_t count) = 0;

        __attribute__((nonnull))
        ssize_t WriteBuf(const void *buf, size_t count)
        {
            ssize_t written = Write(buf, count);

            if (written > 0)
            {
                while (written < count)
                {
                    ssize_t writing = Write((uint8_t *)buf + written, count - written);

                    if (writing <= 0)
                        break;
                    else
                        written += writing;
                }
            }

            return written;
        }

        __attribute__((nonnull))
        bool WriteLn(const char *buf, int len)
        {
            if (buf && len > 0)
            {
                if (Write(buf, len) != len)
                    return false;
            }
            if (Write(LINEBREAK, sizeof(LINEBREAK) - 1) != sizeof(LINEBREAK) - 1)
                return false;
            return true;
        }
    };

    class TInputStream: public TBaseStream
    {
        typedef TBaseStream inherited;

    public:
        using inherited::Read;
        using inherited::ReadBuf;
        using inherited::ReadLn;

        template<typename T> __attribute__((always_inline))
        void ReadValue(T &value)
        {
            ReadBuf(&value, sizeof(value));
        }

    protected:
        virtual ssize_t Write(const void *buf, size_t count)
        {
            return __set_errno(EACCES);
        }
    };

    class TOutputStream: public TBaseStream
    {
        typedef TBaseStream inherited;

    public:
        using inherited::Write;
        using inherited::WriteBuf;
        using inherited::WriteLn;

        template<typename T>
        void WriteValue(const T value)
        {
            WriteBuf(&value, sizeof(value));
        }

    protected:
        virtual ssize_t Read(void *buf, size_t count)
        {
            return __set_errno(EACCES);
        }
    };

    class TStream: public TBaseStream
    {
    private: // types
        typedef TBaseStream inherited;

    public:
        using inherited::Read;
        using inherited::ReadBuf;
        using inherited::ReadLn;

        using inherited::Write;
        using inherited::WriteBuf;
        using inherited::WriteLn;

        template<typename T> __attribute__((always_inline))
        void ReadValue(T &value)
        {
            ReadBuf(&value, sizeof(value));
        }

        template<typename T> __attribute__((always_inline))
        void WriteValue(const T value)
        {
            WriteBuf(&value, sizeof(value));
        }
    };

    class THandleStream: public TStream
    {
    private:
        typedef TStream inherited;

    public:
        THandleStream(int Fd) :
            inherited(), FHandle(Fd)
        {
        }

        virtual ~THandleStream(void)
        {
            // Close();
        }

        void Close()
        {
            if (FHandle)
            {
                ::close(FHandle);
                FHandle = 0;
            }
        }

        int Fd() const
        {
            return FHandle;
        }

    /* TStream */
        __attribute__((nonnull))
        virtual ssize_t Read(void *buf, size_t count, uint32_t timeout = 0)
        {
            /*if (timeout != 0)
            {
                uint32_t recv_length;
                ioctl(FHandle, FIONREAD, &recv_length);

                if (recv_length == 0)
                {
                    struct timeval tv;
                    tv.tv_sec = timeout / 1000;
                    tv.tv_usec = (timeout  % 1000) * 1000;

                    fd_set readfds;
                    FD_ZERO(&readfds);
                    FD_SET(FHandle, &readfds);

                    if (select(1, &readfds, NULL, NULL, &tv) < 1)
                        return __set_errno(ETIMEDOUT);
                }
            }*/
            return ::read(FHandle, buf, count);
        }

        __attribute__((nonnull))
        virtual ssize_t Write(const void *buf, size_t count)
        {
            return ::write(FHandle, (void *)buf, count);
        }

        virtual off_t Position(void)
        {
            return ::lseek(FHandle, 0, SEEK_CUR);
        }

        virtual ssize_t Seek(off_t offset, int origin)
        {
            return ::lseek(FHandle, offset, origin);
        }

    protected:
        int FHandle;
    };
};

#endif

