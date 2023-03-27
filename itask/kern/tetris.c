#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/tetris.h>

// static uint32_t Field[FIELD_HEIGHT][FIELD_WIDTH];

int tetris(void)
{
    bool is_gpu_ready = gpu_ready();
    if (!is_gpu_ready)
        return -E_DEV_RT;

    return 0;
}

#include <kern/traceopt.h>