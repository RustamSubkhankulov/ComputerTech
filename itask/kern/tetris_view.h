#ifndef JOS_KERN_TETRIS_VIEW_H
#define JOS_KERN_TETRIS_VIEW_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <kern/tetris_model.h>
#include <kern/graphics.h>

#define BLOCK_SIZE 32U
#define BLOCK_OUTL_THICKNESS 4U
#define EMPTY_BLOCK_OUTL_THICKNESS 1U

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

void set_up_res(void);
void cleanup_display(void);

void draw_initial(struct Tetris_gamestate* gamestate);

void draw_field_initial(srfc_t* surf, struct Field_elem field[FIELD_HEIGHT][FIELD_WIDTH]);
void draw_logo_initial(srfc_t* surf);
void draw_frame_initial(srfc_t* surf);

void draw_pg_field_elems(srfc_t* surf, struct Field_elem field[FIELD_HEIGHT][FIELD_WIDTH]);
void draw_cur_fig(srfc_t* surf, const struct Figure_info* cur_fig);

void draw_field_elem(srfc_t* surf, pair16_t bl, pair16_t tr, color32bpp_t clr);
void draw_empty_box(srfc_t* surf, pair16_t bl, pair16_t tr);

void fill_empty_figure(srfc_t* surf, pair16_t pos_tl);
void draw_figure(srfc_t* surf, pair16_t pos_tl, const figure_t* fig);

void draw_next_initial(srfc_t* surf);
void draw_hold_initial(srfc_t* surf);
void draw_stat_initial(srfc_t* surf);

void draw_next(srfc_t* surf, figure_type_t next);
void draw_hold(srfc_t* surf, figure_type_t hold);

void fill_empty_next(srfc_t* surf);
void fill_empty_hold(srfc_t* surf);

void draw_score(srfc_t* surf, unsigned score);
void draw_level(srfc_t* surf, unsigned level);
void draw_lines(srfc_t* surf, unsigned lines);

void draw_gameover_msg(srfc_t* surf);

#endif // JOS_KERN_TETRIS_VIEW_H
