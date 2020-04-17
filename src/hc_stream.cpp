#include "hc_stream.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace HiCreation;

TFileStream* TFileStream::Create(const char *path, int flags)
{
    int fd;
    if ((fd = open(path, flags | O_CREAT, 0666)) < 0)
    {
        printf("open %s failed: %d \n", path, errno);
        return NULL;
    }

    TFileStream *file_stream = new TFileStream(fd);
    return file_stream;
}

TPipeStream* TPipeStream::Create(const char *path, TPipeStream::EndPointType type)
{
    if (type == TPipeStream::WRITE)
    {
        if (mkfifo(path, 0666) < 0)
        {
            int errsv = errno;
            if (errsv == EEXIST)
            {
                printf("%s already exists \n", path);
            }
            else
            {
                printf("create %s failed: %d \n", path, errsv);
                return NULL;
            }
        }
    }

    int fd = open(path, type == TPipeStream::WRITE ? O_RDWR : O_RDONLY); // change O_WRONLY to O_RDWR, the open will hanging when no reader
    if (fd == -1)
    {
        printf("open %s failed: %d \n", path, errno);
        return NULL;
    }

    TPipeStream *pipe_stream = new TPipeStream(fd, type);
    return pipe_stream;
}

