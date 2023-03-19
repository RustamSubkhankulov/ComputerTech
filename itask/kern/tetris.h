#ifndef JOS_KERN_TETRIS_H
#define JOS_KERN_TETRIS_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/graphics.h>
#include <kern/gpu.h>

void tetris(void);

#endif // JOS_KERN_TETRIS_H