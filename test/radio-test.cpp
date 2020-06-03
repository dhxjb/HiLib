#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "v4l2_radio.h"

using namespace HiCreation;

static volatile int KeepRunning = 1;

static void IntHandler(int dummy)
{
    printf("%s dummy: %d\n", __func__, dummy);
    KeepRunning = 0;
}

int main(int argc, char **argv)
{
    int ret;
    RADIO_status_t status;
    TV4L2Device *RadioDev;
    TV4L2RadioCtrl *RadioCtrl;

    signal(SIGINT, IntHandler);

    RadioDev = new TV4L2Device("/dev/radio0");
    if (! RadioDev)
    {
        printf("radio dev create err\n");
        return -1;
    }
    RadioCtrl = new TV4L2RadioCtrl(RadioDev);
    if (! RadioCtrl)
    {
        printf("radio ctrl dev create err\n");
        delete RadioDev;
        return -1;
    }

    RadioCtrl->ShutDown();
    usleep(1000);
    if (RadioCtrl->StartUp(RADIO_FM) != 0)
    {
        printf("Radio StartUp failed\n");
        goto ERR_DEV;
    }

    if (RadioCtrl->Tune(90500, &status) != 0)
    {
        printf("Tune freq failed\n");
        goto ERR_DEV;
    }
    
    usleep(1000 * 1000);
    if (RadioCtrl->Seek(SEEK_UP, &status) != 0)
    {
        printf("Seek freq failed \n");
        goto ERR_DEV;
    }

    usleep(1000 * 1000);
    RadioCtrl->ShutDown();
    usleep(1000);

    usleep(100 * 1000);
    if (RadioCtrl->StartUp(RADIO_FM) != 0)
    {
        printf("Radio StartUp failed\n");
        goto ERR_DEV;
    }
    usleep(100 * 1000);
    if (RadioCtrl->Tune(90500, &status) != 0)
    {
        printf("Tune freq failed\n");
        goto ERR_DEV;
    }

    while(KeepRunning)
    {
        usleep(1000);
    }
    printf("Radio test done!\n");

ERR_DEV:
    delete RadioCtrl;
    delete RadioDev;
}