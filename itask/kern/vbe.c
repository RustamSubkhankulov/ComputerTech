#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>

#include <kern/vbe.h>

void vbe_dispi_set_reg(enum Vbe_dispi_reg reg, uint16_t value)
{
    assert(reg <= VBE_DISPI_INDEX_Y_OFFSET);

    outw(VBE_DISPI_INDEX_PORT, (uint16_t) reg);
    outw(VBE_DISPI_DATA_PORT, value);
    return;
}

uint16_t vbe_dispi_get_reg(enum Vbe_dispi_reg reg)
{
    assert(reg <= VBE_DISPI_INDEX_Y_OFFSET);

    outw(VBE_DISPI_INDEX_PORT, (uint16_t) reg);
    return inw(VBE_DISPI_DATA_PORT);
}

bool vbe_dispi_check_bit(enum Vbe_dispi_reg reg, uint8_t bitno)
{
    assert(reg <= VBE_DISPI_INDEX_Y_OFFSET);
    
    return vbe_dispi_get_reg(reg) & (1 << bitno);
}

void vbe_dispi_clear_bit(enum Vbe_dispi_reg reg, uint8_t bitno)
{
    assert(reg <= VBE_DISPI_INDEX_Y_OFFSET);

    uint16_t value = vbe_dispi_get_reg(reg);
    vbe_dispi_set_bit(reg, value & (~(1 << bitno)));
    return;
}

void vbe_dispi_set_bit(enum Vbe_dispi_reg reg, uint8_t bitno)
{
    assert(reg <= VBE_DISPI_INDEX_Y_OFFSET);

    uint16_t value = vbe_dispi_get_reg(reg);
    vbe_dispi_set_bit(reg, value | (1 << bitno));
    return;
}


