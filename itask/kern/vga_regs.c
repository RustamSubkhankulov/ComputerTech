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

void graphics_reg_set_bit(enum Graphic_register reg, uint8_t bitno)
{
    assert(reg >= SET_RESET && reg <= BIT_MASK);

    uint8_t value = get_graphics_reg(reg);
    set_graphics_reg(reg, value | (1 << bitno));
    return;
}

void graphics_reg_clear_bit(enum Graphic_register reg, uint8_t bitno)
{
    assert(reg >= SET_RESET && reg <= BIT_MASK);
    
    uint8_t value = get_graphics_reg(reg);
    set_graphics_reg(reg, value & (~(1 << bitno)));
    return;
}

bool graphics_reg_check_bit(enum Graphic_register reg, uint8_t bitno)
{
    assert(reg >= SET_RESET && reg <= BIT_MASK);

    return (get_graphics_reg(reg) & (1 << bitno));
}

uint8_t graphics_reg_data_rotate_logical_operation_get(void)
{
    return (get_graphics_reg(DATA_ROTATE) >> 3) & 0b11;
}

void graphics_reg_data_rotate_logical_operation_set(uint8_t log_op)
{
    assert(log_op <= 0b11);

    uint8_t data_rotate = get_graphics_reg(DATA_ROTATE);
    set_graphics_reg(DATA_ROTATE, (data_rotate & 0b11100111) | (log_op << 3));
    return;
}

void graphics_reg_data_rotate_rotate_count_set(uint8_t ct)
{
    assert(ct <= 0b111);

    uint8_t data_rotate = get_graphics_reg(DATA_ROTATE);
    set_graphics_reg(DATA_ROTATE, (data_rotate & 0b11111000) | ct);
    return;
}

uint8_t graphics_reg_data_rotate_rotate_count_get(void)
{
    return get_graphics_reg(DATA_ROTATE) & 0b111;
}

void graphics_reg_read_map_select_set(uint8_t rms)
{
    assert(rms <= 0b11);

    uint8_t read_map_select = get_graphics_reg(READ_MAP_SELECT);
    set_graphics_reg(READ_MAP_SELECT, (read_map_select & 0b11111100) | rms);
    return;
}

uint8_t graphics_reg_read_map_select_get(void)
{
    return get_graphics_reg(READ_MAP_SELECT) & 0b11;
}

void graphics_reg_graphics_mode_write_mode_set(uint8_t wmode)
{
    assert(wmode <= 0b11);

    uint8_t graphics_mode = get_graphics_reg(GRAPHICS_MODE);
    set_graphics_reg(GRAPHICS_MODE, (graphics_mode & 0b11111100) | wmode);
}

uint8_t graphics_reg_graphics_mode_write_mode_get(void)
{
    return get_graphics_reg(GRAPHICS_MODE) & 0b11;
}

void graphics_reg_misc_graphics_mem_map_select_set(uint8_t mms)
{
    assert(mms <= 0b11);

    uint8_t misc_graphics = get_graphics_reg(MISC_GRAPHICS);
    set_graphics_reg(MISC_GRAPHICS, (misc_graphics & 0b11110011) | mms);
    return;
}

uint8_t graphics_reg_misc_graphics_mem_map_select_get(void)
{
    return (get_graphics_reg(MISC_GRAPHICS) >> 2) & 0b11;
}

uint8_t graphics_reg_clr_cmp_get(void)
{
    return get_graphics_reg(CLR_CMP) & 0b1111;
}

void graphics_reg_clr_cmp_set(uint8_t clr_cmp)
{
    assert(clr_cmp <= 0b1111);

    uint8_t color_compare = get_graphics_reg(CLR_CMP);
    set_graphics_reg(CLR_CMP, (color_compare & 0b11110000) | clr_cmp);
    return;
}

uint8_t get_sequencer_reg(enum Sequencer_register reg)
{
    assert(reg >= RESET && reg <= SEQ_MEM_MODE);

    outb(SEQ_ADDR_REG, reg);
    return inb(SEQ_DATA_REG);
}

void set_sequencer_reg(enum Sequencer_register reg, uint8_t value)
{
    assert(reg >= RESET && reg <= SEQ_MEM_MODE);

    outb(SEQ_ADDR_REG, reg);
    outb(SEQ_DATA_REG, value);
}

void sequencer_reg_set_bit(enum Sequencer_register reg, uint8_t bitno)
{
    assert(reg >= RESET && reg <= SEQ_MEM_MODE);

    uint8_t value = get_sequencer_reg(reg);
    set_sequencer_reg(reg, value | (1 << bitno));
    return;
}

void sequencer_reg_clear_bit(enum Sequencer_register reg, uint8_t bitno)
{
    assert(reg >= RESET && reg <= SEQ_MEM_MODE);

    uint8_t value = get_sequencer_reg(reg);
    set_sequencer_reg(reg, value & (~(1 << bitno)));
    return;
}

bool sequencer_reg_check_bit(enum Sequencer_register reg, uint8_t bitno)
{
    assert(reg >= RESET && reg <= SEQ_MEM_MODE);

    return (get_sequencer_reg(reg) & (1 << bitno));
}

void sequencer_reg_csas_set(uint8_t csas)
{
    assert(csas <= 0b111);

    if (csas & 0b100)
        sequencer_reg_set_bit(CHAR_MAP_SELECT, CHAR_MAP_SELECT_CSAS2);
    else 
        sequencer_reg_clear_bit(CHAR_MAP_SELECT, CHAR_MAP_SELECT_CSAS2);

    csas &= 0b11;

    uint8_t char_map_select = get_sequencer_reg(CHAR_MAP_SELECT);
    set_sequencer_reg(CHAR_MAP_SELECT, (char_map_select & 0b11110011) | (csas << 2));
    return;
}

uint8_t sequencer_reg_csas_get(void)
{
    uint8_t res = (get_sequencer_reg(CHAR_MAP_SELECT) >> 2) & 0b11;
    if (sequencer_reg_check_bit(CHAR_MAP_SELECT, CHAR_MAP_SELECT_CSAS2)) res |= 0b100;
    return res;
}

void sequencer_reg_csbs_set(uint8_t csbs)
{
    assert(csbs <= 0b111);

    if (csbs & 0b100)
        sequencer_reg_set_bit(CHAR_MAP_SELECT, CHAR_MAP_SELECT_CSBS2);
    else 
        sequencer_reg_clear_bit(CHAR_MAP_SELECT, CHAR_MAP_SELECT_CSBS2);

    csbs &= 0b11;

    uint8_t char_map_select = get_sequencer_reg(CHAR_MAP_SELECT);
    set_sequencer_reg(CHAR_MAP_SELECT, (char_map_select & 0b11111100) | csas);
    return;
}

uint8_t sequencer_reg_csbs_get(void)
{
    uint8_t res = get_sequencer_reg(CHAR_MAP_SELECT) & 0b11;
    if (sequencer_reg_check_bit(CHAR_MAP_SELECT, CHAR_MAP_SELECT_CSBS2)) res |= 0b100;
    return res;
}

uint8_t get_attr_ctrl_reg(enum Attr_ctrl_register reg)
{
    inb(INP_STATUS_1_REG_READ);
    outb(ATTR_ADDR_DATA_REG, reg & 0b11111);
    return inb(ATTR_DATA_READ_REG);
}

void set_attr_ctrl_reg(enum Attr_ctrl_register reg, uint8_t value)
{
    assert(reg >= PALETTE_0 && reg <= CLR_SELECT);

    inb(INP_STATUS_1_REG_READ);
    outb(ATTR_ADDR_DATA_REG, reg & 0b11111);
    outb(ATTR_ADDR_DATA_REG, value);
}

void attr_ctrl_reg_set_bit(enum Attr_ctrl_register reg, uint8_t bitno)
{
    assert(reg >= PALETTE_0 && reg <= CLR_SELECT);

    uint8_t value = get_attr_ctrl_reg(reg);
    set_attr_ctrl_reg(reg, value | (1 << bitno));
    return;
}

void attr_ctrl_reg_clear_bit(enum Attr_ctrl_register reg, uint8_t bitno)
{
    assert(reg >= PALETTE_0 && reg <= CLR_SELECT);
    
    uint8_t value = get_attr_ctrl_reg(reg);
    set_attr_ctrl_reg(reg, value & (~(1 << bitno)));
    return;
}

bool attr_ctrl_reg_check_bit(enum Attr_ctrl_register reg, uint8_t bitno)
{
    assert(reg >= PALETTE_0 && reg <= CLR_SELECT);

    return (get_attr_ctrl_reg(reg) & (1 << bitno));
}

void attr_ctrl_plt_reg_int_ind_set(uint8_t pltno, uint8_t ind)
{
    assert(pltno <= 0x0F);
    assert(ind <= 0b11111);
    
    enum Attr_ctrl_register reg = PALETTE_0 + pltno;
    uint8_t plt = get_attr_ctrl_reg(reg);
    set_attr_ctrl_reg(reg, (plt & 0b11000000) | ind);
    return;
}

uint8_t attr_ctrl_plt_reg_int_ind_get(uint8_t pltno)
{
    assert(pltno <= 0x0F);

    enum Attr_ctrl_register reg = PALETTE_0 + pltno;
    return get_attr_ctrl_reg(reg) & 0b111111;
}

void attr_ctrl_reg_hor_pxl_pan_set(uint8_t psc)
{
    assert(psc <= 0b1111);

    uint8_t hor_pxl_pan = get_attr_ctrl_reg(HOR_PXL_PAN);
    set_attr_ctrl_reg(HOR_PXL_PAN, (hor_pxl_pan & 0b11110000) | psc);
    return;
}

uint8_t attr_ctrl_reg_hor_pxl_pan_get(void)
{
    return get_attr_ctrl_reg(HOR_PXL_PAN) & 0b1111;
}



