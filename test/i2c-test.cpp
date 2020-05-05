#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "hc_i2c.h"

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
    TI2CDev *I2CDev;
    uint8_t bus_no = 0;

    signal(SIGINT, IntHandler);

    I2CDev = new TI2CDev(bus_no, 0x39);
    if (ret = I2CDev->Open() < 0)
    {
        printf("i2c %s open failed: %d \n", I2CDev->Name(), ret);
        goto END;
    }

    while(KeepRunning)
    {
        usleep(1000);
    }

END:
    delete I2CDev;
    return ret;
}