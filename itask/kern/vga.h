#ifndef JOS_KERN_VGA_H
#define JOS_KERN_VGA_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/pci.h>
#include <kern/pmap.h>

void init_vga(void);
void test_vga(void);

#endif // JOS_KERN_VGA_H