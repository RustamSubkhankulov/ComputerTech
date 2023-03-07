#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/vga.h>
#include <inc/string.h>

void init_vga(void)
{
    if (trace_vga)
        cprintf("vga: initialization stated. \n");


    if (trace_vga)
        cprintf("vga: initialization finished. \n");

    return;
}

void test_vga(void)
{
    if (trace_vga)
        cprintf("vga: tests started \n");

    if (trace_vga)
        cprintf("vga: tests finished \n");

    return;
}
