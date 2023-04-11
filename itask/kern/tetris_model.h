#ifndef JOS_KERN_TETRIS_MODEL_H
#define JOS_KERN_TETRIS_MODEL_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/graphics.h>

#include <kern/gpu.h>
#include <kern/vbe.h>
#include <kern/ktimer.h>

#define FIELD_WIDTH  10
#define FIELD_HEIGHT 20

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
    HOLD   = 5
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

struct Field_elem
{
    bool present;
    color32bpp_t color;
};

struct Figure_info
{
    figure_type_t type;
    pair16_t pos_tl;
    unsigned rot;
};

struct Tetris_gamestate
{
    bool game_is_on;
    bool gamestate_updated;

    struct Field_elem field[FIELD_HEIGHT][FIELD_WIDTH];

    unsigned score;
    bool score_updated;

    unsigned level;
    bool level_updated;

    unsigned lines;
    bool lines_updated;

    struct Figure_info cur_fig;
    bool cur_fig_updated;

    figure_type_t next;
    bool next_updated;

    figure_type_t hold;
    bool hold_updated;

    uint64_t frames_per_cell;

    bool pg_updated;
    bool use_hold;
};

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

int tetris(void);

#endif // JOS_KERN_TETRIS_MODEL_H