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
    set_graphics_reg(MISC_GRAPHICS, (misc_graphics & 0b11110011) | (mms << 2));
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

    if (csas & (1 << 3))
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
    if (sequencer_reg_check_bit(CHAR_MAP_SELECT, CHAR_MAP_SELECT_CSAS2)) res |= (1 << 3);
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

uint8_t get_crtc_reg(enum CRTC_register reg)
{
    assert(reg >= HOR_TOTAL && reg <= LINE_CMP);

    outb(CRTC_CTRL_ADDR_REG, reg);
    return inb(CRTC_CTRL_DATA_REG);
}

void set_crtc_reg(enum CRTC_register reg, uint8_t value)
{
    assert(reg >= HOR_TOTAL && reg <= LINE_CMP);

    outb(CRTC_CTRL_ADDR_REG, reg);
    outb(CRTC_CTRL_DATA_REG, value);
}

void crtc_reg_set_bit(enum CRTC_register reg, uint8_t bitno)
{
    assert(reg >= HOR_TOTAL && reg <= LINE_CMP);

    uint8_t value = get_crtc_reg(reg);
    set_crtc_reg(reg, value | (1 << bitno));
    return;
}

void crtc_reg_clear_bit(enum CRTC_register reg, uint8_t bitno)
{
    assert(reg >= HOR_TOTAL && reg <= LINE_CMP);

    uint8_t value = get_crtc_reg(reg);
    set_crtc_reg(reg, value & (~(1 << bitno)));
    return;
}

bool crtc_reg_check_bit(enum CRTC_register reg, uint8_t bitno)
{
    assert(reg >= HOR_TOTAL && reg <= LINE_CMP);

    return get_crtc_reg(reg) & (1 << bitno);
}

void ctrc_reg_end_hor_blank_set(uint8_t ehb)
{
    assert(ehb <= 0b111111);

    if (ehb & (1 << 5))
        crtc_reg_set_bit(END_HOR_RETRACE, END_HOR_RETRACE_EHB5);
    else 
        crtc_reg_clear_bit(END_HOR_RETRACE, END_HOR_RETRACE_EHB5);

    ehb &= 0b11111;

    uint8_t end_hor_blank = get_crtc_reg(END_HOR_BLANK);
    set_crtc_reg(END_HOR_BLANK, (end_hor_blank & 0b11100000) | ehb);
    return;
}

uint8_t ctrc_reg_end_hor_blank_get(void)
{
    uint8_t ehb = get_crtc_reg(END_HOR_BLANK) & 0b11111;
    if (crtc_reg_check_bit(END_HOR_RETRACE, END_HOR_RETRACE_EHB5)) ehb |= (1 << 5);

    return ehb;
}

void ctrc_reg_display_enable_skew_set(uint8_t des)
{
    assert(des <= 0b11);

    uint8_t end_hor_blank = get_crtc_reg(END_HOR_BLANK);
    set_crtc_reg(END_HOR_BLANK, (end_hor_blank & 0b10011111) | (des << 5));
    return;
}

uint8_t ctrc_reg_display_enable_skew_get(void)
{
    return (get_crtc_reg(END_HOR_BLANK) >> 5) & 0b11;
}

void ctrc_reg_end_hor_retrace_set(uint8_t ehr)
{
    assert(ehr <= 0b11111);

    uint8_t end_hor_retrace = get_crtc_reg(END_HOR_RETRACE);
    set_crtc_reg(END_HOR_RETRACE, (end_hor_retrace & 0b11100000) | ehr);
    return;
}

uint8_t ctrc_reg_end_hor_retrace_get(void)
{
    return get_crtc_reg(END_HOR_RETRACE) & 0b11111;
}

void ctrc_reg_hor_retrace_skew_set(uint8_t hrs)
{
    assert(hrs <= 0b11);

    uint8_t end_hor_retrace = get_crtc_reg(END_HOR_RETRACE);
    set_crtc_reg(END_HOR_RETRACE, (end_hor_retrace & 0b10011111) | (hrs << 5));
    return;
}

uint8_t ctrc_reg_hor_retrace_skew_get(void)
{
    return (get_crtc_reg(END_HOR_RETRACE) >> 5) & 0b11;
}

void ctrc_reg_byte_panning_set(uint8_t bp)
{
    assert(bp <= 0b11);

    uint8_t preset_raw_scan = get_crtc_reg(PRESET_RAW_SCAN);
    set_crtc_reg(PRESET_RAW_SCAN, (preset_raw_scan & 0b10011111) | (bp << 5));
}

uint8_t ctrc_reg_byte_panning_get(void)
{
    return (get_crtc_reg(PRESET_RAW_SCAN) >> 5) & 0b11;
}

void ctrc_reg_preset_raw_scan_set(uint8_t prs)
{
    assert(prs <= 0b11111);

    uint8_t preset_raw_scan = get_crtc_reg(PRESET_RAW_SCAN);
    set_crtc_reg(PRESET_RAW_SCAN, (preset_raw_scan & 0b11100000) | prs);
}

uint8_t ctrc_reg_preset_raw_scan_get(void)
{
    return get_crtc_reg(PRESET_RAW_SCAN) & 0b11111;
}

void crtc_reg_max_scan_line_set(uint8_t msl)
{
    assert(msl <= 0b11111);

    uint8_t max_scan_line = get_crtc_reg(MAX_SCAN_LINE);
    set_crtc_reg(MAX_SCAN_LINE, (max_scan_line & 0b11100000) | msl);
    return;
}

uint8_t crtc_reg_max_scan_line_get(void)
{
    return get_crtc_reg(MAX_SCAN_LINE) & 0b11111;
}

void crtc_reg_cursor_scan_line_start_set(uint8_t csls)
{
    assert(csls <= 0b11111);

    uint8_t cursor_start = get_crtc_reg(CURSOR_START);
    set_crtc_reg(CURSOR_START, (cursor_start & 0b11100000) | csls);
    return;
}

uint8_t crtc_reg_cursor_scan_line_start_get(void)
{
    return get_crtc_reg(CURSOR_START) & 0b11111;
}

void crtc_reg_cursor_scew_set(uint8_t cs)
{
    assert(cs <= 0b11);

    uint8_t cursor_end = get_crtc_reg(CURSOR_END);
    set_crtc_reg(CURSOR_END, (cursor_end & 0b10011111) | (cs << 5));
    return;
}

uint8_t crtc_reg_cursor_scew_get(void)
{
    return (get_crtc_reg(CURSOR_END) >> 5) & 0b11;
}

void crtc_reg_cursor_scan_line_end_set(uint8_t csle)
{
    assert(csle <= 0b11111);

    uint8_t cursor_end = get_crtc_reg(CURSOR_END);
    set_crtc_reg(CURSOR_END, (cursor_end & 0b11100000) | csle);
    return;
}

uint8_t crtc_reg_cursor_scan_line_end_get(void)
{
    return get_crtc_reg(CURSOR_END) & 0b11111;
}

void crtc_reg_start_addr_set(uint16_t sa)
{
    set_crtc_reg(START_ADDR_HIGH, (uint8_t) ((sa >> 8) & 0b11111111));
    set_crtc_reg(START_ADDR_LOW, (uint8_t) (sa & 0b11111111));
    return;
}

uint16_t crtc_reg_start_addr_get(void)
{
    uint16_t high = (uint16_t) get_crtc_reg(START_ADDR_HIGH);
    uint16_t low  = (uint16_t) get_crtc_reg(START_ADDR_LOW);
    return (low & 0b11111111) | ((high & 0b11111111) << 8);
}

void crtc_reg_cursor_loc_set(uint16_t cl)
{
    set_crtc_reg(CURSOR_LOC_HIGH, (uint8_t) ((cl >> 8) & 0b11111111));
    set_crtc_reg(CURSOR_LOC_LOW, (uint8_t) (cl & 0b11111111));
    return;
}

uint16_t crtc_reg_cursor_loc_get(void)
{
    uint16_t high = (uint16_t) get_crtc_reg(CURSOR_LOC_HIGH);
    uint16_t low  = (uint16_t) get_crtc_reg(CURSOR_LOC_LOW);
    return (low & 0b11111111) | ((high & 0b11111111) << 8);
}

void crtc_reg_ver_retrace_end_set(uint8_t vre)
{
    assert(vre <= 0b1111);

    uint8_t ver_retrace_end = get_crtc_reg(VER_RETRACE_END);
    set_crtc_reg(VER_RETRACE_END, (ver_retrace_end & 0b11110000) | vre);
}

uint8_t crtc_reg_ver_retrace_end_get(void)
{
    return get_crtc_reg(VER_RETRACE_END) & 0b1111;
}

void crtc_reg_underline_loc_set(uint8_t ul)
{
    assert(ul <= 0b11111);

    uint8_t underline_loc = get_crtc_reg(UNDERLINE_LOC);
    set_crtc_reg(UNDERLINE_LOC, (underline_loc & 0b11100000) | ul);
    return;
}

uint8_t crtc_reg_underline_loc_get(void)
{
    return get_crtc_reg(UNDERLINE_LOC) & 0b11111;
}

void crtc_reg_ver_total_set(uint16_t vt)
{
    assert(vt <= 0b1111111111);

    if (vt & (1 << 9))
        crtc_reg_set_bit(OVERFLOW, OVERFLOW_VT9)
    else 
        crtc_reg_clear_bit(OVERFLOW, OVERFLOW_VT9);

    if (vt & (1 << 8))
        crtc_reg_set_bit(OVERFLOW, OVERFLOW_VT8)
    else 
        crtc_reg_clear_bit(OVERFLOW, OVERFLOW_VT8);

    set_crtc_reg(VER_TOTAL, (uint8_t) vt);
    return;
}

uint16_t crtc_reg_ver_total_get(void)
{
    uint16_t vt = (uint16_t) get_crtc_reg(VER_TOTAL);
    if (crtc_reg_check_bit(OVERFLOW, OVERFLOW_VT8)) vt |= (1 << 8);
    if (crtc_reg_check_bit(OVERFLOW, OVERFLOW_VT9)) vt |= (1 << 9);

    return vt;
}

void crtc_reg_ver_retrace_start_set(uint16_t vrs)
{
    assert(vrs <= 0b1111111111);

    if (vrs & (1 << 9))
        crtc_reg_set_bit(OVERFLOW, OVERFLOW_VRS9);
    else 
        crtc_reg_clear_bit(OVERFLOW, OVERFLOW_VRS9);

    if (vrs & (1 << 8))
        crtc_reg_set_bit(OVERFLOW, OVERFLOW_VRS8);
    else 
        crtc_reg_clear_bit(OVERFLOW, OVERFLOW_VRS8);

    set_crtc_reg(VER_RETRACE_START, (uint8_t) vrs);
    return;
}

uint16_t crtc_reg_ver_retrace_start_get(void)
{
    uint16_t vrs = (uint16_t) get_crtc_reg(VER_RETRACE_START);
    if (crtc_reg_check_bit(OVERFLOW, OVERFLOW_VRS8)) vrs |= (1 << 8);
    if (crtc_reg_check_bit(OVERFLOW, OVERFLOW_VRS9)) vrs |= (1 << 9);

    return vrs;
}

void crtc_reg_ver_display_end_set(uint16_t vde)
{
    assert(vde <= 0b1111111111);

    if (vde & (1 << 9))
        crtc_reg_set_bit(OVERFLOW, OVERFLOW_VDE9);
    else 
        crtc_reg_clear_bit(OVERFLOW, OVERFLOW_VDE9);

    if (vde & (1 << 8))
        crtc_reg_set_bit(OVERFLOW, OVERFLOW_VDE8);
    else 
        crtc_reg_clear_bit(OVERFLOW, OVERFLOW_VDE8);

    set_crtc_reg(VER_DISPLAY_END, (uint8_t) vde);
    return;
}

uint16_t crtc_reg_ver_display_end_get(void)
{
    uint16_t vde = (uint16_t) get_crtc_reg(VER_DISPLAY_END);
    if (crtc_reg_check_bit(OVERFLOW, OVERFLOW_VDE8)) vde |= (1 << 8);
    if (crtc_reg_check_bit(OVERFLOW, OVERFLOW_VDE9)) vde |= (1 << 9);

    return vde;
}

void crtc_reg_start_ver_blank_set(uint16_t svb)
{
    assert(svb <= 0b1111111111);

    if (svb & (1 << 9))
        crtc_reg_set_bit(MAX_SCAN_LINE, MAX_SCAN_LINE_SVB9);
    else 
        crtc_reg_clear_bit(MAX_SCAN_LINE, MAX_SCAN_LINE_SVB9);

    if (svb & (1 << 8))
        crtc_reg_set_bit(OVERFLOW, OVERFLOW_SVB8);
    else 
        crtc_reg_clear_bit(OVERFLOW, OVERFLOW_SVB8);

    set_crtc_reg(START_VER_BLANK, (uint8_t) svb);
    return;
}

uint16_t crtc_reg_start_ver_blank_get(void)
{
    uint16_t svb = (uint16_t) get_crtc_reg(START_VER_BLANK);
    if (crtc_reg_check_bit(OVERFLOW, OVERFLOW_SVB8))           svb |= (1 << 8);
    if (crtc_reg_check_bit(MAX_SCAN_LINE, MAX_SCAN_LINE_SVB9)) svb |= (1 << 9);

    return svb;
}
void crtc_reg_end_ver_blank_set(uint8_t evb)
{
    assert(evb <= 0b1111111);

    uint8_t end_ver_blank = get_crtc_reg(END_VER_BLANK);
    set_crtc_reg(END_VER_BLANK, (end_ver_blank & 0b10000000) | evb);
    return;
}

uint8_t crtc_reg_end_ver_blank_get(void)
{
    return get_crtc_reg(END_VER_BLANK) & 0b1111111;
}

void crtc_reg_line_cmp_set(uint16_t lc)
{
    assert(lc <= 0b1111111111);

    if (lc & (1 << 9))
        crtc_reg_set_bit(MAX_SCAN_LINE, MAX_SCAN_LINE_LC9);
    else 
        crtc_reg_clear_bit(MAX_SCAN_LINE, MAX_SCAN_LINE_LC9);

    if (lc & (1 << 8))
        crtc_reg_set_bit(OVERFLOW, OVERFLOW_LC8);
    else 
        crtc_reg_clear_bit(OVERFLOW, OVERFLOW_LC8);

    set_crtc_reg(LINE_CMP, (uint8_t) lc);
    return;
}

uint16_t crtc_reg_line_cmp_get(void)
{
    uint16_t lc = (uint16_t) get_crtc_reg(LINE_CMP);
    if (crtc_reg_check_bit(OVERFLOW, OVERFLOW_LC8))           lc |= (1 << 8);
    if (crtc_reg_check_bit(MAX_SCAN_LINE, MAX_SCAN_LINE_LC9)) lc |= (1 << 9);

    return lc;
}

void write_color_to_palette(unsigned entry, uint32_t color)
{
    color_reg_dac_addr_wmode_set(entry);

    uint8_t r = (uint8_t) (color);
    uint8_t g = (uint8_t) (color >> 8);
    uint8_t b = (uint8_t) (color >> 16);

    color_reg_dac_data_set(r);
    color_reg_dac_data_set(g);
    color_reg_dac_data_set(b);

    return;
}

uint32_t read_color_from_palette(unsigned entry)
{
    color_reg_dac_addr_rmode_set(entry);
    
    uint8_t r = color_reg_dac_data_get(); 
    uint8_t g = color_reg_dac_data_get();
    uint8_t b = color_reg_dac_data_get();

    return ((uint32_t) r) 
         | ((uint32_t) g >> 8)
         | ((uint32_t) b >> 16); 
}

void misc_output_set_bit(uint8_t bitno)
{
    uint8_t misc_output = inb(MISC_OUTPUT_R);
    outb(MISC_OUTPUT_W, misc_output | (1<< bitno));
    return;
}

void misc_output_clear_bit(uint8_t bitno)
{
    uint8_t misc_output = inb(MISC_OUTPUT_R);
    outb(MISC_OUTPUT_W, misc_output & (~(1<< bitno)));
    return;
}

bool misc_output_check_bit(uint8_t bitno)
{
    return inb(MISC_OUTPUT_R) & (1 << bitno);
}

void feat_ctrl_set_bit(uint8_t bitno)
{
    uint8_t feat_ctrl = inb(FEAT_CTRL_R);
    outb(FEAT_CTRL_W, feat_ctrl | (1 << bitno));
    return;
}

void feat_ctrl_clear_bit(uint8_t bitno)
{
    uint8_t feat_ctrl = inb(FEAT_CTRL_R);
    outb(FEAT_CTRL_W, feat_ctrl & (~(1 << bitno)));
    return;
}

bool feat_ctrl_check_bit(uint8_t bitno)
{
    return inb(FEAT_CTRL_R) & (1 << bitno);
}

bool inp_status_0_check_bit(uint8_t bitno)
{
    return inb(INP_STATUS_0) & (1 << bitno);
}

bool inp_status_1_check_bit(uint8_t bitno)
{
    return inb(INP_STATUS_1) & (1 << bitno);
}

void misc_output_clock_select_set(uint8_t cs)
{
    uint8_t misc_output = inb(MISC_OUTPUT_R);
    outb(MISC_OUTPUT_W, (misc_output & 0b111100) | (cs << 2));
    return;
}

uint8_t misc_output_clock_select_get(void)
{
    return (inb(MISC_OUTPUT_R) >> 2) & 0b11;
}