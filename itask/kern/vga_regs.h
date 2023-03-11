#ifndef JOS_KERN_VGA_REGS_H
#define JOS_KERN_VGA_REGS_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/vga_ports.h>

//--------------------
// Graphics Registers
//--------------------

enum Graphic_register
{
    SET_RESET            = 0x0,
    ENABLE_SET_RESET     = 0x1,
    COLOR_COMPARE        = 0x2,
    DATA_ROTATE          = 0x3,
    READ_MAP_SELECT      = 0x4, 
    GRAPHICS_MODE        = 0x5,
    MISC_GRAPHICS        = 0x6,
    COLOR_LOL_DIDNT_CARE = 0x7,
    BIT_MASK             = 0x8
};

uint8_t get_graphics_reg(enum Graphic_register reg);
void set_graphics_reg(enum Graphic_register reg, uint8_t value);

void graphics_reg_set_reset_set_bit(uint8_t bitno);
void graphics_reg_set_reset_clear_bit(uint8_t bitno);

void graphics_reg_enable_set_reset_set_bit(uint8_t bitno);
void graphics_reg_enable_set_reset_clear_bit(uint8_t bitno);

#endif // JOS_KERN_VGA_REGS_H
