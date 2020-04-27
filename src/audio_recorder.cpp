#include <stdio.h>
#include <string.h>

#include "audio_recorder.h"

using namespace HiCreation;

TAudioRecorder::TAudioRecorder(const char *name)
{
    FStream = TFileStream::Create(name, O_WRONLY);
    if (! FStream)
        printf("file: %s stream create failed\n", name);
}

void TAudioRecorder::OnFrame(audio_frame_t *frame)
{
    if (FStream)
        FStream->WriteBuf(frame->buf, frame->len);
}
