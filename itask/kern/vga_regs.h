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
    SET_RESET          = 0x00,
    ENABLE_SET_RESET   = 0x01,
    CLR_CMP            = 0x02,
    DATA_ROTATE        = 0x03,
    READ_MAP_SELECT    = 0x04, 
    GRAPHICS_MODE      = 0x05,
    MISC_GRAPHICS      = 0x06,
    CLR_LOL_DIDNT_CARE = 0x07,
    BIT_MASK           = 0x08
};

#define GRAPHICS_MODE_RMODE     3U
#define GRAPHICS_MODE_HOST_OE   4U
#define GRAPHICS_MODE_SHIFT_REG 5U
#define GRAPHICS_MODE_SHIFT_256 6U

#define MISC_GRAPHICS_CHAIN_OE  1U
#define MISC_GRAPHICS_ALPHA_DIS 0U

uint8_t get_graphics_reg(enum Graphic_register reg);
void    set_graphics_reg(enum Graphic_register reg, uint8_t value);

void graphics_reg_set_bit(enum Graphic_register reg, uint8_t bitno);
void graphics_reg_clear_bit(enum Graphic_register reg, uint8_t bitno);
bool graphics_reg_check_bit(enum Graphic_register reg, uint8_t bitno);

uint8_t graphics_reg_clr_cmp_get(void);
void    graphics_reg_clr_cmp_set(uint8_t clr_cmp);

// 0b00 - no op, 0b01 - AND, 0b10 - OR, 0b11 - XOR
enum Data_rotate_log_op
{
    NOP = 0b00,
    AND = 0b01,
    OR  = 0b10,
    XOR = 0b11
};

void    graphics_reg_data_rotate_logical_operation_set(uint8_t log_op);
uint8_t graphics_reg_data_rotate_logical_operation_get(void);

void    graphics_reg_data_rotate_rotate_count_set(uint8_t ct);
uint8_t graphics_reg_data_rotate_rotate_count_get(void);

void    graphics_reg_read_map_select_set(uint8_t rms);
uint8_t graphics_reg_read_map_select_get(void);

void    graphics_reg_graphics_mode_write_mode_set(uint8_t wmode);
uint8_t graphics_reg_graphics_mode_write_mode_get(void);

// 00b -- A0000h-BFFFFh (128K region) // 01b -- A0000h-AFFFFh (64K region)
// 10b -- B0000h-B7FFFh (32K region)  // 11b -- B8000h-BFFFFh (32K region)
enum Misc_graphics_mem_map
{
    REGION_128_KB_A0000H = 0b00,
    REGION_64_KB_A0000H  = 0b01,
    REGION_32_KB_B0000H  = 0b10,
    REGION_32_KB_B8000H  = 0b11
};

void    graphics_reg_misc_graphics_mem_map_select_set(uint8_t mms);
uint8_t graphics_reg_misc_graphics_mem_map_select_get(void);

static inline void graphics_reg_bit_mask_set(uint8_t bm)
{
    set_graphics_reg(BIT_MASK, bm);
}

static inline uint8_t graphics_reg_bit_mask_get(void)
{
    return get_graphics_reg(BIT_MASK);
}

//---------------------
// Sequencer Registers
//---------------------

enum Sequencer_register
{
    RESET           = 0x00,
    CLOCKING_MODE   = 0x01,
    MAP_MASK        = 0x02,
    CHAR_MAP_SELECT = 0x03,
    SEQ_MEM_MODE    = 0x04
};

#define RESET_SR 1
#define RESET_AR 0

#define CLOCKING_MODE_SD   5U
#define CLOCKING_MODE_S4   4U
#define CLOCKING_MODE_DCR  3U
#define CLOCKING_MODE_SLR  2U
#define CLOCKING_MODE_98DM 0U // 0 - 9 dots/char, 1 - 8 dots/char 

#define CHAR_MAP_SELECT_CSAS2 5U
#define CHAR_MAP_SELECT_CSBS2 4U

#define SEQ_MEM_MODE_CHAIN_4 3U
#define SEQ_MEM_MODE_OE_DIS  2U
#define SEQ_MEM_MODE_EXT_MEM 1U

uint8_t get_sequencer_reg(enum Sequencer_register reg);
void    set_sequencer_reg(enum Sequencer_register reg, uint8_t value);

void sequencer_reg_set_bit(enum Sequencer_register reg, uint8_t bitno);
void sequencer_reg_clear_bit(enum Sequencer_register reg, uint8_t bitno);
bool sequencer_reg_check_bit(enum Sequencer_register reg, uint8_t bitno);

// 000b -- Select font residing at 0000h - 1FFFh 001b -- Select font residing at 4000h - 5FFFh
// 010b -- Select font residing at 8000h - 9FFFh 011b -- Select font residing at C000h - DFFFh
// 100b -- Select font residing at 2000h - 3FFFh 101b -- Select font residing at 6000h - 7FFFh
// 110b -- Select font residing at A000h - BFFFh 111b -- Select font residing at E000h - FFFFh

void    sequencer_reg_csas_set(uint8_t csas);
uint8_t sequencer_reg_csas_get(void);

void    sequencer_reg_csbs_set(uint8_t csbs);
uint8_t sequencer_reg_csbs_get(void);

//--------------------------------
// Attribute Controller Registers
//--------------------------------

enum Attr_ctrl_register
{
    PALETTE_0      = 0x00,
    PALETTE_1      = 0x01,
    PALETTE_2      = 0x02,
    PALETTE_3      = 0x03,
    PALETTE_4      = 0x04,
    PALETTE_5      = 0x05,
    PALETTE_6      = 0x06,
    PALETTE_7      = 0x07,
    PALETTE_8      = 0x08,
    PALETTE_9      = 0x09,
    PALETTE_A      = 0x0A,
    PALETTE_B      = 0x0B,
    PALETTE_C      = 0x0C,
    PALETTE_D      = 0x0D,
    PALETTE_E      = 0x0E,
    PALETTE_F      = 0x0F,
    ATTR_MODE_CTRL = 0x10,
    OVERSCAN_CLR   = 0x11,
    CLR_PLANE_E    = 0x12,
    HOR_PXL_PAN    = 0x13,
    CLR_SELECT     = 0x14,
};

#define ATTR_MODE_CTRL_P54S  7
#define ATTR_MODE_CTRL_8BIT  6
#define ATTR_MODE_CTRL_PPM   5
#define ATTR_MODE_CTRL_BLINK 3
#define ATTR_MODE_CTRL_LGE   2
#define ATTR_MODE_CTRL_MONO  1
#define ATTR_MODE_CTRL_ATGE  0

uint8_t get_attr_ctrl_reg(enum Attr_ctrl_register reg);
void    set_attr_ctrl_reg(enum Attr_ctrl_register reg, uint8_t value);

void attr_ctrl_reg_set_bit(enum Attr_ctrl_register reg, uint8_t bitno);
void attr_ctrl_reg_clear_bit(enum Attr_ctrl_register reg, uint8_t bitno);
bool attr_ctrl_reg_check_bit(enum Attr_ctrl_register reg, uint8_t bitno);

void    attr_ctrl_reg_plt_int_ind_set(uint8_t pltno, uint8_t ind);
uint8_t attr_ctrl_reg_plt_int_ind_get(uint8_t pltno);

void    attr_ctrl_reg_hor_pxl_pan_set(uint8_t psc);
uint8_t attr_ctrl_reg_hor_pxl_pan_get(void);

static inline void attr_ctrl_reg_overscan_clr_set(uint8_t oc)
{
    set_attr_ctrl_reg(OVERSCAN_CLR, oc);
}

static inline uint8_t attr_ctrl_reg_overscan_clr_get(void)
{
    return get_attr_ctrl_reg(OVERSCAN_CLR);
}

//----------------
// CRTC Registers
//----------------

enum CRTC_register
{
    HOR_TOTAL         = 0x00,
    END_HOR_DISPLAY   = 0x01,
    START_HOR_BLANK   = 0x02,
    END_HOR_BLANK     = 0x03,
    START_HOR_RETRACE = 0x04,
    END_HOR_RETRACE   = 0x05,
    VER_TOTAL         = 0x06,
    OVERFLOW          = 0x07,
    PRESET_RAW_SCAN   = 0x08,
    MAX_SCAN_LINE     = 0x09,
    CURSOR_START      = 0x0A, 
    CURSOR_END        = 0x0B,
    START_ADDR_HIGH   = 0x0C,
    START_ADDR_LOW    = 0x0D,
    CURSOR_LOC_HIGH   = 0x0E,
    CURSOR_LOC_LOW    = 0x0F,
    VER_RETRACE_START = 0x10,
    VER_RETRACE_END   = 0x11,
    VER_DISPLAY_END   = 0x12,
    OFFSET            = 0x13,
    UNDERLINE_LOC     = 0x14,
    START_VER_BLANK   = 0x15,
    END_VER_BLANK     = 0x16,
    CRTC_MODE_CTRL    = 0x17,
    LINE_CMP          = 0x18
};

#define END_HOR_BLANK_EVRA 7

#define END_HOR_RETRACE_EHB5 7

#define OVERFLOW_VRS9 7
#define OVERFLOW_VDE9 6
#define OVERFLOW_VT9  5
#define OVERFLOW_LC8  4
#define OVERFLOW_SVB8 3
#define OVERFLOW_VRS8 2
#define OVERFLOW_VDE8 1
#define OVERFLOW_VT8  0

#define MAX_SCAN_LINE_SD   7
#define MAX_SCAN_LINE_LC9  6
#define MAX_SCAN_LINE_SVB9 5

#define CURSOR_START_CD 5

#define VER_RETRACE_END_PROTECT   7
#define VER_RETRACE_END_BANDWIDTH 6

#define UNDERLINE_LOC_DW   6
#define UNDERLINE_LOC_DIV4 5

#define CRTC_MODE_CTRL_SE    7
#define CRTC_MODE_CTRL_WB    6
#define CRTC_MODE_CTRL_AW    4
#define CRTC_MODE_CTRL_DIV2  3
#define CRTC_MODE_CTRL_SLDIV 2
#define CRTC_MODE_CTRL_MAP14 1
#define CRTC_MODE_CTRL_MAP13 0

uint8_t get_crtc_reg(enum CRTC_register reg);
void    set_crtc_reg(enum CRTC_register reg, uint8_t value);

void crtc_reg_set_bit(enum CRTC_register reg, uint8_t bitno);
void crtc_reg_clear_bit(enum CRTC_register reg, uint8_t bitno);
bool crtc_reg_check_bit(enum CRTC_register reg, uint8_t bitno);

void    ctrc_reg_end_hor_blank_set(uint8_t ehb);
uint8_t ctrc_reg_end_hor_blank_get(void);

void    ctrc_reg_display_enable_skew_set(uint8_t des);
uint8_t ctrc_reg_display_enable_skew_get(void);

void    ctrc_reg_end_hor_retrace_set(uint8_t ehr);
uint8_t ctrc_reg_end_hor_retrace_get(void);

void    ctrc_reg_hor_retrace_skew_set(uint8_t hrs);
uint8_t ctrc_reg_hor_retrace_skew_get(void);

void    ctrc_reg_byte_panning_set(uint8_t bp);
uint8_t ctrc_reg_byte_panning_get(void);

void    ctrc_reg_preset_raw_scan_set(uint8_t prs);
uint8_t ctrc_reg_preset_raw_scan_get(void);

void    crtc_reg_max_scan_line_set(uint8_t msl);
uint8_t crtc_reg_max_scan_line_get(void);

void    crtc_reg_cursor_scan_line_start_set(uint8_t csls);
uint8_t crtc_reg_cursor_scan_line_start_get(void);

void    crtc_reg_cursor_scew_set(uint8_t cs);
uint8_t crtc_reg_cursor_scew_get(void);

void    crtc_reg_cursor_scan_line_end_set(uint8_t csle);
uint8_t crtc_reg_cursor_scan_line_end_get(void);

void     crtc_reg_start_addr_set(uint16_t sa);
uint16_t crtc_reg_start_addr_get(void);

void     crtc_reg_cursor_loc_set(uint16_t cl);
uint16_t crtc_reg_cursor_loc_get(void);

void    crtc_reg_ver_retrace_end_set(uint8_t vre);
uint8_t crtc_reg_ver_retrace_end_get(void);

void    crtc_reg_underline_loc_set(uint8_t ul);
uint8_t crtc_reg_underline_loc_get(void);

void    crtc_reg_end_ver_blank_set(uint8_t evb);
uint8_t crtc_reg_end_ver_blank_get(void);

void     crtc_reg_ver_total_set(uint16_t vt);
uint16_t crtc_reg_ver_total_get(void);

void     crtc_reg_ver_retrace_start_set(uint16_t vrs);
uint16_t crtc_reg_ver_retrace_start_get(void);

void     crtc_reg_ver_display_end_set(uint16_t vde);
uint16_t crtc_reg_ver_display_end_get(void);

void     crtc_reg_start_ver_blank_set(uint16_t svb);
uint16_t crtc_reg_start_ver_blank_get(void);

void     crtc_reg_line_cmp_set(uint16_t lc);
uint16_t crtc_reg_line_cmp_get(void);

static inline void crtc_reg_hor_total_set(uint8_t ht)
{
    set_crtc_reg(HOR_TOTAL, ht);
}

static inline uint8_t crtc_reg_hor_total_get(void)
{
    return get_crtc_reg(HOR_TOTAL);
}

static inline void crtc_reg_end_hor_display_set(uint8_t ehd)
{
    set_crtc_reg(END_HOR_DISPLAY, ehd);
}

static inline uint8_t crtc_reg_end_hor_display_get(void)
{
    return get_crtc_reg(END_HOR_DISPLAY);
}

static inline void crtc_reg_start_hor_blank_set(uint8_t shb)
{
    set_crtc_reg(START_HOR_BLANK, shb);
}

static inline uint8_t crtc_reg_start_hor_blank_get(void)
{
    return get_crtc_reg(START_HOR_BLANK);
}

static inline void crtc_reg_start_hor_retrace_set(uint8_t shr)
{
    set_crtc_reg(START_HOR_RETRACE, shr);
}

static inline uint8_t crtc_reg_start_hor_retrace_get(void)
{
    return get_crtc_reg(START_HOR_RETRACE);
}

static inline void crtc_reg_offset_set(uint8_t offs)
{
    set_crtc_reg(OFFSET, offs);
}

static inline uint8_t crtc_reg_offset_get(void)
{
    return get_crtc_reg(OFFSET);
}

//-----------------
// Color Registers
//-----------------

enum Color_register
{
    DAC_ADDR_WMODE = DAC_ADDR_WRITE_MODE_REG,      // rw
    DAC_ADDR_RMODE = DAC_ADDR_READ_MODE_REG_WRITE, // -w
    DAC_DATA       = DAC_DATA_REG,                 // rw
    DAC_STATE      = DAC_STATE_REG_READ            // r-
};

enum DAC_state
{
    DAC_ACCEPT_READS  = 0b00,
    DAC_ACCEPT_WRITES = 0b11
};

static inline void color_reg_dac_addr_wmode_set(uint8_t addr)
{
    outb(DAC_ADDR_WMODE, addr);
}

static inline uint8_t color_reg_dac_addr_wmode_get(void)
{
    return inb(DAC_ADDR_WMODE);
}

static inline void color_reg_dac_addr_rmode_set(uint8_t addr)
{
    outb(DAC_ADDR_RMODE, addr);
}

static inline void color_reg_dac_data_set(uint8_t value)
{
    outb(DAC_DATA, value);
}

static inline uint8_t color_reg_dac_data_get(void)
{
    return inb(DAC_DATA);
}

/*

Each color - i-i+5 bits, i=0,8,16
 ________ ________ ________ ________
|________|________|________|________|
0        8        16       24      31
    red     green    blue    unused 

*/

void write_color_to_palette(unsigned entry, uint32_t color);
uint32_t read_color_from_palette(unsigned entry);

//--------------------
// External Registers
//--------------------

enum External_register
{
    MISC_OUTPUT_R = MISC_OUTPUT_REG_READ,  // r-
    MISC_OUTPUT_W = MISC_OUTPUT_REG_WRITE, // -w
    FEAT_CTRL_R   = FEAT_CTRL_REG_READ,    // r-
    FEAT_CTRL_W   = FEAT_CTRL_REG_WRITE,   // -w
    INP_STATUS_0  = INP_STATUS_0_REG_READ, // r- 
    INP_STATUS_1  = INP_STATUS_1_REG_READ  // r-
};

#define MISC_OUTPUT_VSYNCP  7
#define MISC_OUTPUT_HSYNCP  6 
#define MISC_OUTPUT_OE_PAGE 5
#define MISC_OUTPUT_RAM_EN  1
#define MISC_OUTPUT_IOAS    0

void misc_output_set_bit(uint8_t bitno);
void misc_output_clear_bit(uint8_t bitno);
bool misc_output_check_bit(uint8_t bitno);

void    misc_output_clock_select_set(uint8_t cs);
uint8_t misc_output_clock_select_get(void);

#define FEAT_CTRL_FC1 1
#define FEAT_CTRL_FC0 0

void feat_ctrl_set_bit(uint8_t bitno);
void feat_ctrl_clear_bit(uint8_t bitno);
bool feat_ctrl_check_bit(uint8_t bitno);

#define INP_STATUS_0_SS       4

bool inp_status_0_check_bit(uint8_t bitno);

#define INP_STATUS_1_VRETRACE 3
#define INP_STATUS_1_DD       0

bool inp_status_1_check_bit(uint8_t bitno);

#endif // JOS_KERN_VGA_REGS_H
