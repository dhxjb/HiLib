#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

static volatile int KeepRunning = 1;

static void IntHandler(int dummy)
{
    printf("%s dummy: %d\n", __func__, dummy);
    KeepRunning = 0;
}

int main(int argc, char **argv)
{
    int ret;

    signal(SIGINT, IntHandler);

    while(KeepRunning)
    {
        usleep(1000);
    }
}