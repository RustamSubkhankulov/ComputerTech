#include <unistd.h>
#include <stdio.h>
#include "pids.h"

//---------------------------------------------------------

int print_pids(void)
{

    printf("PID: %d PPID: %d \n", getpid(), getppid());

    return 0;
}
