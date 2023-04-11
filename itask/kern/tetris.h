#ifndef JOS_KERN_TETRIS_H
#define JOS_KERN_TETRIS_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/graphics.h>

#include <kern/gpu.h>
#include <kern/vbe.h>
#include <kern/graphics.h>
#include <kern/ktimer.h>

static const enum TimerType Timer_type = HPET0;

static const uint64_t Tetris_framerate = 30U;
static const int64_t Timer_timeout_ms = 1000 / Tetris_framerate;

#define TETRIS_LEVELS_NUM 15U
static const float Tetris_speed_table[TETRIS_LEVELS_NUM] = { 0.016670f, 0.021017f, 0.026977f,
                                                             0.035256f, 0.046930f, 0.063610f,
                                                             0.087900f, 0.123600f, 0.177500f, 
                                                             0.259800f, 0.388000f, 0.590000f,
                                                             0.920000f, 1.460000f, 2.360000f };

#define INITIAL_LEVEL    1U
#define LINES_PER_LEVEL 10U

enum Tetris_ctrl_key
{
    NO_ACTION = 0,
    DOWN   = 1,
    RIGHT  = 2,
    LEFT   = 3,
    ROT    = 4,
    HOLD   = 5,
};

typedef enum Figure_type
{
    NONE    = -1,
    I_BLOCK = +0,
    J_BLOCK = +1,
    L_BLOCK = +2,
    O_BLOCK = +3,
    S_BLOCK = +4,
    T_BLOCK = +5,
    Z_BLOCK = +6,
    N_BLOCK_TYPES,

} figure_type_t;

typedef enum Figure_pos
{
    DEG_0 = 0,
    DEG_90 = 1,
    DEG_180 = 2,
    DEG_270 = 3,
    N_BLOCK_ROT,

} figure_pos_t;

#define FIGURE_SIZE 4

typedef struct Figure
{
    figure_type_t type;
    color32bpp_t color;
    char map[FIGURE_SIZE][FIGURE_SIZE];

} figure_t;

const static figure_t Figures[N_BLOCK_TYPES][N_BLOCK_ROT] = 
{
    {
        {.type = I_BLOCK, .color.rgb = 0x0001FFFF, .map = {{0, 1, 0, 0}, 
                                                           {0, 1, 0, 0}, 
                                                           {0, 1, 0, 0}, 
                                                           {0, 1, 0, 0}} },

        {.type = I_BLOCK, .color.rgb = 0x0001FFFF, .map = {{0, 0, 0, 0},
                                                           {1, 1, 1, 1},
                                                           {0, 0, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = I_BLOCK, .color.rgb = 0x0001FFFF, .map = {{0, 0, 1, 0}, 
                                                           {0, 0, 1, 0}, 
                                                           {0, 0, 1, 0}, 
                                                           {0, 0, 1, 0}} },

        {.type = I_BLOCK, .color.rgb = 0x0001FFFF, .map = {{0, 0, 0, 0},
                                                           {0, 0, 0, 0},
                                                           {1, 1, 1, 1},
                                                           {0, 0, 0, 0}} }
    },  
    {
        {.type = J_BLOCK, .color.rgb = 0x001035AC, .map = {{0, 1, 1, 0},
                                                           {0, 1, 0, 0},
                                                           {0, 1, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = J_BLOCK, .color.rgb = 0x001035AC, .map = {{0, 0, 0, 0},
                                                           {0, 1, 1, 1},
                                                           {0, 0, 0, 1},
                                                           {0, 0, 0, 0}} },

        {.type = J_BLOCK, .color.rgb = 0x001035AC, .map = {{0, 0, 0, 0},
                                                           {0, 0, 1, 0},
                                                           {0, 0, 1, 0},
                                                           {0, 1, 1, 0}} },

        {.type = J_BLOCK, .color.rgb = 0x001035AC, .map = {{0, 0, 0, 0},
                                                           {1, 0, 0, 0},
                                                           {1, 1, 1, 0},
                                                           {0, 0, 0, 0}} }
    },
    {
        {.type = L_BLOCK, .color.rgb = 0x00ED7014, .map = {{1, 1, 0, 0},
                                                           {0, 1, 0, 0},
                                                           {0, 1, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = L_BLOCK, .color.rgb = 0x00ED7014, .map = {{0, 0, 0, 1},
                                                           {0, 1, 1, 1},
                                                           {0, 0, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = L_BLOCK, .color.rgb = 0x00ED7014, .map = {{0, 0, 0, 0},
                                                           {0, 0, 1, 0},
                                                           {0, 0, 1, 0},
                                                           {0, 0, 1, 1}} },

        {.type = L_BLOCK, .color.rgb = 0x00ED7014, .map = {{0, 0, 0, 0},
                                                           {0, 0, 0, 0},
                                                           {1, 1, 1, 0},
                                                           {1, 0, 0, 0}} }
    },
    {
        {.type = O_BLOCK, .color.rgb = 0x00FAFD0F, .map = {{0, 0, 0, 0},
                                                           {0, 1, 1, 0},
                                                           {0, 1, 1, 0},
                                                           {0, 0, 0, 0}} },

        {.type = O_BLOCK, .color.rgb = 0x00FAFD0F, .map = {{0, 0, 0, 0},
                                                           {0, 1, 1, 0},
                                                           {0, 1, 1, 0},
                                                           {0, 0, 0, 0}} },

        {.type = O_BLOCK, .color.rgb = 0x00FAFD0F, .map = {{0, 0, 0, 0},
                                                           {0, 1, 1, 0},
                                                           {0, 1, 1, 0},
                                                           {0, 0, 0, 0}} },

        {.type = O_BLOCK, .color.rgb = 0x00FAFD0F, .map = {{0, 0, 0, 0},
                                                           {0, 1, 1, 0},
                                                           {0, 1, 1, 0},
                                                           {0, 0, 0, 0}} }
    },
    {
        {.type = S_BLOCK, .color.rgb = 0x0037FD12, .map = {{1, 0, 0, 0},
                                                           {1, 1, 0, 0},
                                                           {0, 1, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = S_BLOCK, .color.rgb = 0x0037FD12, .map = {{0, 0, 1, 1},
                                                           {0, 1, 1, 0},
                                                           {0, 0, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = S_BLOCK, .color.rgb = 0x0037FD12, .map = {{0, 0, 0, 0},
                                                           {0, 0, 1, 0},
                                                           {0, 0, 1, 1},
                                                           {0, 0, 0, 1}} },

        {.type = S_BLOCK, .color.rgb = 0x0037FD12, .map = {{0, 0, 0, 0},
                                                           {0, 0, 0, 0},
                                                           {0, 1, 1, 0},
                                                           {1, 1, 0, 0}} }
    },
    {
        {.type = T_BLOCK, .color.rgb = 0x00D34DD2, .map = {{0, 1, 0, 0},
                                                           {1, 1, 1, 0},
                                                           {0, 0, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = T_BLOCK, .color.rgb = 0x00D34DD2, .map = {{0, 0, 1, 0} ,
                                                           {0, 0, 1, 1} ,
                                                           {0, 0, 1, 0} ,
                                                           {0, 0, 0, 0} } },

        {.type = T_BLOCK, .color.rgb = 0x00D34DD2, .map = {{0, 0, 0, 0},
                                                           {0, 0, 0, 0},
                                                           {0, 1, 1, 1},
                                                           {0, 0, 1, 0}} },

        {.type = T_BLOCK, .color.rgb = 0x00D34DD2, .map = {{0, 0, 0, 0},
                                                           {0, 1, 0, 0},
                                                           {1, 1, 0, 0},
                                                           {0, 1, 0, 0}} }
    },
    {
        {.type = Z_BLOCK, .color.rgb = 0x00E3242B, .map = {{0, 1, 0, 0},
                                                           {1, 1, 0, 0},
                                                           {1, 0, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = Z_BLOCK, .color.rgb = 0x00E3242B, .map = {{0, 1, 1, 0},
                                                           {0, 0, 1, 1},
                                                           {0, 0, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = Z_BLOCK, .color.rgb = 0x00E3242B, .map = {{0, 0, 0, 0},
                                                           {0, 0, 0, 1},
                                                           {0, 0, 1, 1},
                                                           {0, 0, 1, 0}} },

        {.type = Z_BLOCK, .color.rgb = 0x00E3242B, .map = {{0, 0, 0, 0},
                                                           {0, 0, 0, 0},
                                                           {1, 1, 0, 0},
                                                           {0, 1, 1, 0}} }
    }
};

#define BLOCK_SIZE 32U
#define BLOCK_OUTL_THICKNESS 4U
#define EMPTY_BLOCK_OUTL_THICKNESS 1U

#define FIELD_WIDTH  10
#define FIELD_HEIGHT 20

#define BLOCK_NUM ((FIELD_WIDTH) * (FIELD_HEIGHT))

#define FIELD_BAR_OUTL_THICKNESS 5U
#define FIELD_BAR_WIDTH  (2 * (FIELD_BAR_OUTL_THICKNESS) + (FIELD_WIDTH ) * (BLOCK_SIZE)) 
#define FIELD_BAR_HEIGHT (2 * (FIELD_BAR_OUTL_THICKNESS) + (FIELD_HEIGHT) * (BLOCK_SIZE))


#define STAT_WIDTH  8
#define STAT_HEIGHT 5

#define STAT_BAR_OUTL_THICKNESS 5U
#define STAT_BAR_WIDTH  (2 * (STAT_BAR_OUTL_THICKNESS) + (STAT_WIDTH ) * (BLOCK_SIZE)) 
#define STAT_BAR_HEIGHT (2 * (STAT_BAR_OUTL_THICKNESS) + (STAT_HEIGHT) * (BLOCK_SIZE))

#define STAT_TEXT_BPP 3U


#define NEXT_WIDTH  8
#define NEXT_HEIGHT 6

#define NEXT_BAR_OUTL_THICKNESS 5U
#define NEXT_BAR_WIDTH  (2 * (NEXT_BAR_OUTL_THICKNESS) + (NEXT_WIDTH ) * (BLOCK_SIZE)) 
#define NEXT_BAR_HEIGHT (2 * (NEXT_BAR_OUTL_THICKNESS) + (NEXT_HEIGHT) * (BLOCK_SIZE))

#define NEXT_TEXT_BPP 3U


#define HOLD_WIDTH  8
#define HOLD_HEIGHT 6

#define HOLD_BAR_OUTL_THICKNESS 5U
#define HOLD_BAR_WIDTH  (2 * (HOLD_BAR_OUTL_THICKNESS) + (HOLD_WIDTH ) * (BLOCK_SIZE)) 
#define HOLD_BAR_HEIGHT (2 * (HOLD_BAR_OUTL_THICKNESS) + (HOLD_HEIGHT) * (BLOCK_SIZE))

#define HOLD_TEXT_BPP 3U

const static color32bpp_t Bg_clr             = {.rgb = 0x000F0F33};

const static color32bpp_t Empty_box_clr      = {.rgb = 0x00080819};
const static color32bpp_t Empty_box_outl_clr = {.rgb = 0x00333333};

const static color32bpp_t Field_bar_outl_clr = {.rgb = 0x0000FAFF};
const static color32bpp_t Frame_block_clr    = {.rgb = 0x00686266};

const static color32bpp_t Stat_bar_outl_clr = {.rgb = 0x0000FAFF};
const static color32bpp_t Stat_bar_bg_clr   = {.rgb = 0x00080819};

const static color32bpp_t Next_bar_outl_clr = {.rgb = 0x0000FAFF};
const static color32bpp_t Next_bar_bg_clr   = {.rgb = 0x00080819};

const static color32bpp_t Hold_bar_outl_clr = {.rgb = 0x0000FAFF};
const static color32bpp_t Hold_bar_bg_clr   = {.rgb = 0x00080819};

const static color32bpp_t Red_color         = {.rgb = 0x00E3242B};
const static color32bpp_t White_color       = {.rgb = 0x00FFFFFF};
const static color32bpp_t Black_color       = {.rgb = 0x00000000};

int tetris(void);

#endif // JOS_KERN_TETRIS_H