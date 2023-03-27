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

    void draw_frame();
    void run_loop();

    return 0;
}

void draw_frame(void)
{
    
}

void draw_next(void)
{

}

void draw_score(void)
{

}

void run_loop(void)
{

}
