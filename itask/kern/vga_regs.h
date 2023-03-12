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
void    graphics_reg_misc_graphics_mem_map_select_set(uint8_t mms);
uint8_t graphics_reg_misc_graphics_mem_map_select_get(void);

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

#endif // JOS_KERN_VGA_REGS_H
