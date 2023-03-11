#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/vga_regs.h>

uint8_t get_graphics_reg(enum Graphic_register reg)
{
    assert(reg >= SET_RESET && reg <= BIT_MASK);

    outb(GRAPH_CTRL_ADDR_REG, reg);
    return inb(GRAPH_CTRL_DATA_REG);
}

void set_graphics_reg(enum Graphic_register reg, uint8_t value)
{
    assert(reg >= SET_RESET && reg <= BIT_MASK);

    outb(GRAPH_CTRL_ADDR_REG, reg);
    outb(GRAPH_CTRL_DATA_REG, value);
}

void graphics_reg_set_reset_set_bit(uint8_t bitno)
{
    assert(bitno < 4);

    uint8_t set_reset = get_graphics_reg(SET_RESET);
    set_graphics_reg(SET_RESET, set_reset | (1 << bitno));
}

void graphics_reg_set_reset_clear_bit(uint8_t bitno)
{
    assert(bitno < 4);

    uint8_t set_reset = get_graphics_reg(SET_RESET);
    set_graphics_reg(SET_RESET, set_reset & ((~(1 << bitno)) & 0b1111));
}

void graphics_reg_enable_set_reset_set_bit(uint8_t bitno)
{
    assert(bitno < 4);

    uint8_t enable_set_reset = get_graphics_reg(ENABLE_SET_RESET);
    set_graphics_reg(ENABLE_SET_RESET, enable_set_reset | (1 << bitno));
}

void graphics_reg_enable_set_reset_clear_bit(uint8_t bitno)
{
    assert(bitno < 4);

    uint8_t enable_set_reset = get_graphics_reg(ENABLE_SET_RESET);
    set_graphics_reg(ENABLE_SET_RESET, enable_set_reset & ((~(1 << bitno)) & 0b1111));
}