#ifndef JOS_KERN_TETRIS_H
#define JOS_KERN_TETRIS_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/graphics.h>

#include <kern/gpu.h>
#include <kern/vbe.h>
#include <kern/graphics.h>

#define FIELD_WIDTH  10
#define FIELD_HEIGHT 20

typedef enum Figure_type
{
    I_BLOCK = 0,
    J_BLOCK = 1,
    L_BLOCK = 2,
    O_BLOCK = 3,
    S_BLOCK = 4,
    T_BLOCK = 5,
    Z_BLOCK = 6,
    N_BLOCK_TYPES,

} figure_type_t;

typedef enum Figure_pos
{
    DEG_0 = 0,
    DEG_90 = 1,
    DEG_180 = 2,
    DEG_270 = 3,
    N_BLOCK_POS,

} figure_pos_t;

typedef struct Figure
{
    figure_type_t type;
    color32bpp_t color;
    char map[4][4];

} figure_t;

const static figure_t Figures[N_BLOCK_TYPES][N_BLOCK_POS] = 
{
    {
        {.type = I_BLOCK, .color.rgb = 0x00006464, .map = {{0, 1, 0, 0}, 
                                                           {0, 1, 0, 0}, 
                                                           {0, 1, 0, 0}, 
                                                           {0, 1, 0, 0}} },

        {.type = I_BLOCK, .color.rgb = 0x00006464, .map = {{0, 0, 0, 0},
                                                           {1, 1, 1, 1},
                                                           {0, 0, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = I_BLOCK, .color.rgb = 0x00006464, .map = {{0, 0, 1, 0}, 
                                                           {0, 0, 1, 0}, 
                                                           {0, 0, 1, 0}, 
                                                           {0, 0, 1, 0}} },

        {.type = I_BLOCK, .color.rgb = 0x00006464, .map = {{0, 0, 0, 0},
                                                           {0, 0, 0, 0},
                                                           {1, 1, 1, 1},
                                                           {0, 0, 0, 0}} }
    },  
    {
        {.type = J_BLOCK, .color.rgb = 0x0000FF00, .map = {{0, 1, 1, 0},
                                                           {0, 1, 0, 0},
                                                           {0, 1, 0, 0},
                                                           {0, 0, 0, 0}} },

        {.type = J_BLOCK, .color.rgb = 0x0000FF00, .map = {{0, 0, 0, 0},
                                                           {0, 1, 1, 1},
                                                           {0, 0, 0, 1},
                                                           {0, 0, 0, 0}} },

        {.type = J_BLOCK, .color.rgb = 0x0000FF00, .map = {{0, 0, 0, 0},
                                                           {0, 0, 1, 0},
                                                           {0, 0, 1, 0},
                                                           {0, 1, 1, 0}} },

        {.type = J_BLOCK, .color.rgb = 0x0000FF00, .map = {{0, 0, 0, 0},
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

const static color32bpp_t Bg_clr      = {.rgb = 0x000A3463};
const static color32bpp_t Outline_clr = {.rgb = 0x0000FAFF};

int tetris(void);

void draw_frame(void);
void draw_next(void);
void draw_score(void);

void run_loop(void);

#endif // JOS_KERN_TETRIS_H