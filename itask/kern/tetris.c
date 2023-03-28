#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>
#include <inc/string.h>

#include <kern/tetris.h>

static pair16_t Field_tl;

static pair16_t Score_pos;
static pair16_t Level_pos;
static pair16_t Lines_pos;

static struct Field_elem
{
    bool present;
    color32bpp_t color;

} Field[FIELD_HEIGHT][FIELD_WIDTH];

static struct Tetris_gamestate
{
    unsigned score;
    unsigned level;
    unsigned lines;

} Gamestate;

static void set_up_res(void);
static void draw_initial(void);

static void draw_field(srfc_t* surf);
static void draw_logo(srfc_t* surf);
static void draw_frame(srfc_t* surf);

static void draw_field_elems(srfc_t* surf);
static void draw_field_elem(srfc_t* surf, pair16_t bl, pair16_t tr, color32bpp_t clr);
static void draw_empty_box(srfc_t* surf, pair16_t bl, pair16_t tr);

static void draw_next(srfc_t* surf);
static void draw_hold(srfc_t* surf);
static void draw_score(srfc_t* surf);

static void set_score(uint32_t score, srfc_t* surf);
static void set_level(uint32_t level, srfc_t* surf);
static void set_lines(uint32_t lines, srfc_t* surf);

static void run_loop(void);

int tetris(void)
{
    bool is_gpu_ready = gpu_ready();
    if (is_gpu_ready == false)
        return -E_DEV_RT;

    set_up_res();
    draw_initial();

    Gamestate.level = 1;
    Gamestate.score = 0;
    Gamestate.lines = 0;

    run_loop();

    return 0;
}

static void set_up_res(void)
{
    pair16_t res = gpu_get_display_res();

    if (res.x != Resolution.x || res.y != Resolution.y)
    {
        int err = gpu_set_display_res(Resolution);
        if (err < 0) panic("gpu_set_display_res(): %i \n", err);
    }
}

static void draw_initial()
{
    srfc_t surf = { 0 };

    int err = gpu_request_surface(&surf, 0);
    if (err < 0) panic("gpu_request_surface(): %i \n", err);

    srfc_clear(&surf);
    srfc_fill(&surf, Bg_clr);

    draw_field(&surf);
    draw_frame(&surf);
    draw_logo(&surf);

    draw_score(&surf);
    draw_hold(&surf);
    draw_next(&surf);

    set_score(0xDEAD, &surf);
    set_level(0xBEBE, &surf);
    set_lines(0xBABA, &surf);

    err = gpu_submit_surface(&surf);
    if (err < 0) panic("gpu_submit_surface(): %i \n", err);

    err = gpu_page_flip();
    if (err < 0) panic("gpu_page_flip(): %i \n", err);
}

static void draw_field(srfc_t* surf)
{
    pair16_t center = {.x = surf->res.x / 2, .y = surf->res.y / 2};

    pair16_t bl = {.x = center.x - FIELD_BAR_WIDTH / 2, .y = center.y + FIELD_BAR_HEIGHT / 2};
    pair16_t tr = {.x = center.x + FIELD_BAR_WIDTH / 2, .y = center.y - FIELD_BAR_HEIGHT / 2};

    srfc_box_thick_in(surf, bl ,tr, Field_bar_outl_clr, FIELD_BAR_OUTL_THICKNESS);

    Field_tl.x = center.x - FIELD_BAR_WIDTH  / 2 + FIELD_BAR_OUTL_THICKNESS; 
    Field_tl.y = center.y - FIELD_BAR_HEIGHT / 2 + FIELD_BAR_OUTL_THICKNESS;

    Field[0][0].present = true;
    Field[0][0].color = Figures[S_BLOCK][3].color;

    draw_field_elems(surf);
}

static void draw_field_elems(srfc_t* surf)
{
    pair16_t bl = {.x = Field_tl.x, .y = Field_tl.y + BLOCK_SIZE};
    pair16_t tr = {.x = Field_tl.x + BLOCK_SIZE, .y = Field_tl.y};

    for (unsigned y = 0; y < FIELD_HEIGHT; y++)
    {
        for (unsigned x = 0; x < FIELD_WIDTH; x++)
        {
            if (Field[y][x].present == false)
                draw_empty_box(surf, bl, tr);
            else 
                draw_field_elem(surf, bl, tr, Field[y][x].color);
        
            bl.x += BLOCK_SIZE;
            tr.x += BLOCK_SIZE;
        }

        bl.y += BLOCK_SIZE;
        tr.y += BLOCK_SIZE;
    
        bl.x = Field_tl.x;
        tr.x = Field_tl.x + BLOCK_SIZE;
    }
}

// TODO: make if beautiful
static void draw_field_elem(srfc_t* surf, pair16_t bl, pair16_t tr, color32bpp_t clr)
{
    color32bpp_t border_clr = {.r = (uint8_t) ((float) clr.r * 0.8),
                               .g = (uint8_t) ((float) clr.g * 0.8),
                               .b = (uint8_t) ((float) clr.b * 0.8)};

    srfc_bar(surf, bl, tr, border_clr);

    color32bpp_t line_clr = {.r = (uint8_t) ((float) clr.r * 0.6),
                             .g = (uint8_t) ((float) clr.g * 0.6),
                             .b = (uint8_t) ((float) clr.b * 0.6)};

    pair16_t br = {.x = tr.x, .y = bl.y};
    pair16_t tl = {.x = bl.x, .y = tr.y};

    srfc_line(surf, bl, tr, line_clr);
    srfc_line(surf, br, tl, line_clr);

    bl.x += BLOCK_OUTL_THICKNESS;
    bl.y -= BLOCK_OUTL_THICKNESS; 

    tr.x -= BLOCK_OUTL_THICKNESS;
    tr.y += BLOCK_OUTL_THICKNESS;

    srfc_bar(surf, bl, tr, clr);
    srfc_box_thick_in(surf, bl ,tr, line_clr, 1);
}

static void draw_empty_box(srfc_t* surf, pair16_t bl, pair16_t tr)
{
    srfc_bar(surf, bl, tr, Empty_box_clr);
    srfc_box_thick_in(surf, bl ,tr, Empty_box_outl_clr, EMPTY_BLOCK_OUTL_THICKNESS);
}

static void draw_logo(srfc_t* surf)
{
    const char* title = "TETRIS";
    unsigned title_len = strlen(title);

    uint8_t  ppb = 6; 
    uint16_t pixel_per_sym = 8 * ppb;

    color32bpp_t colors[6] = {Figures[Z_BLOCK][0].color,
                              Figures[L_BLOCK][0].color,
                              Figures[O_BLOCK][0].color,
                              Figures[S_BLOCK][0].color,
                              Figures[I_BLOCK][0].color,
                              Figures[T_BLOCK][0].color};

    pair16_t sym_pos = {.x = Field_tl.x / 2 - title_len * pixel_per_sym / 2 + BLOCK_SIZE / 2, 
                        .y = surf->res.y / 2 - pixel_per_sym};

    const uint16_t outl_thickness = 4;

    uint16_t mid_y = sym_pos.y + pixel_per_sym / 2 - 5;
    uint16_t mid_x = sym_pos.x + title_len * pixel_per_sym / 2 - 2;

    pair16_t bl = {.x = mid_x - 3 * pixel_per_sym - outl_thickness,
                   .y = mid_y + pixel_per_sym + outl_thickness};

    pair16_t tr = {.x = mid_x + 3 * pixel_per_sym - outl_thickness,
                   .y = mid_y - pixel_per_sym + outl_thickness};

    srfc_bar(surf, bl, tr, Empty_box_clr);
    srfc_box_thick_in(surf, bl ,tr, Field_bar_outl_clr, outl_thickness);

    pair16_t p1 = {.x = mid_x - pixel_per_sym - outl_thickness, 
                   .y = mid_y + 3 * pixel_per_sym + outl_thickness};

    pair16_t p2 = {.x = p1.x + 2 * pixel_per_sym + outl_thickness,
                   .y = p1.y - 2 * pixel_per_sym - outl_thickness};

    srfc_bar(surf, p1, p2, Empty_box_clr);
    srfc_box_thick_in(surf, p1, p2, Field_bar_outl_clr, outl_thickness);

    for (unsigned iter = 0; iter < title_len; iter++)
    {
        srfc_putchar(surf, title[iter], sym_pos, colors[iter], ppb);

        sym_pos.x += pixel_per_sym;
    }

    color32bpp_t darker_yellow = {.rgb = 0x008EAC1E};
    pair16_t num_pos = {.x = p1.x - 40, .y = p2.y + 20};
    srfc_puts(surf, "100", num_pos, darker_yellow, 8);

    color32bpp_t yellow = {.rgb = 0x00EEFC5E};
    num_pos.x = p1.x - 45; 
    num_pos.y = p2.y + 15;
    srfc_puts(surf, "100", num_pos, yellow, 8);    

    color32bpp_t white = {.rgb = 0x00FFFFFF};
    const char* author = "by Rustamchik (2023)";
    pair16_t author_pos = {.x = surf->res.x - 8 * 2 * strlen(author) - BLOCK_SIZE,
                           .y = surf->res.y - 8 * 2 - BLOCK_SIZE};
    srfc_puts(surf, author, author_pos, white, 2);
}

static void draw_frame(srfc_t* surf)
{
    pair16_t up_bl = {.x = 0, .y = BLOCK_SIZE - 1};
    pair16_t up_tr = {.x = BLOCK_SIZE - 1, .y = 0};

    pair16_t down_bl = {.x = 0, .y = surf->res.y - 1};
    pair16_t down_tr = {.x = BLOCK_SIZE - 1, .y = surf->res.y - BLOCK_SIZE - 1};

    for (unsigned iter = 0; iter < surf->res.x / BLOCK_SIZE; iter++)
    {
        draw_field_elem(surf, up_bl, up_tr, Frame_block_clr);
        draw_field_elem(surf, down_bl, down_tr, Frame_block_clr);

        up_bl.x += BLOCK_SIZE;
        up_tr.x += BLOCK_SIZE;

        down_bl.x += BLOCK_SIZE;
        down_tr.x += BLOCK_SIZE;
    }

    pair16_t left_bl = {.x = 0, .y = BLOCK_SIZE - 1};
    pair16_t left_tr = {.x = BLOCK_SIZE - 1, .y = 0};

    pair16_t right_bl = {.x = surf->res.x - BLOCK_SIZE - 1, .y = BLOCK_SIZE - 1};
    pair16_t right_tr = {.x = surf->res.x - 1, .y = 0};

    for (unsigned iter = 0; iter < surf->res.y / BLOCK_SIZE; iter++)
    {
        draw_field_elem(surf, left_bl, left_tr, Frame_block_clr);
        draw_field_elem(surf, right_bl, right_tr, Frame_block_clr);

        left_bl.y += BLOCK_SIZE;
        left_tr.y += BLOCK_SIZE;

        right_bl.y += BLOCK_SIZE;
        right_tr.y += BLOCK_SIZE;
    }
}

static void draw_next(srfc_t* surf)
{

}

static void draw_score(srfc_t* surf)
{
    pair16_t score_bar_tl = {.x = Field_tl.x + FIELD_BAR_WIDTH + BLOCK_SIZE - FIELD_BAR_OUTL_THICKNESS,
                             .y = Field_tl.y - FIELD_BAR_OUTL_THICKNESS};

    pair16_t score_bar_bl = {.x = score_bar_tl.x, .y = score_bar_tl.y + SCORE_BAR_HEIGHT};
    pair16_t score_bar_tr = {.x = score_bar_tl.x + SCORE_BAR_WIDTH, .y = score_bar_tl.y};

    srfc_bar(surf, score_bar_bl, score_bar_tr, Score_bar_bg_clr);
    srfc_box_thick_in(surf, score_bar_bl, score_bar_tr, Score_bar_outl_clr, SCORE_BAR_OUTL_THICKNESS);

    color32bpp_t white = {.rgb = 0x00FFFFFF};

    score_bar_bl.x += BLOCK_SIZE / 2;

    const uint8_t ppb = SCORE_TEXT_BPP;
    const unsigned string_num = 3;

    const char* string[3] = {"LINES:", "LEVEL:", "SCORE:"};
    pair16_t*   coords[3] = {&Lines_pos, &Level_pos, &Score_pos};

    pair16_t num_pos = {.x = score_bar_bl.x + BLOCK_SIZE * 4 + BLOCK_SIZE / 2,
                        .y = score_bar_bl.y};

    for (unsigned iter = 0; iter < string_num; iter++)
    {
        score_bar_bl.y -= BLOCK_SIZE + BLOCK_SIZE / 2;
        srfc_puts(surf, string[iter], score_bar_bl, white, ppb);
    
        num_pos.y -= BLOCK_SIZE + BLOCK_SIZE / 2;
        *(coords[iter]) = num_pos;
        srfc_puts(surf, "0000", num_pos, white, ppb);
    }
}

static void set_score(uint32_t score, srfc_t* surf)
{
    if (Gamestate.score == score)
        return;

    Gamestate.score = score;

    char str[5] = { 0 };
    color32bpp_t white = {.rgb = 0x00FFFFFF};

    srfc_puts(surf, "\b\b\b\b", Score_pos, Score_bar_bg_clr, SCORE_TEXT_BPP);
    srfc_puts(surf, itoa(score, str, 16), Score_pos, white, SCORE_TEXT_BPP);
}

static void set_level(uint32_t level, srfc_t* surf)
{
    if (Gamestate.level == level)
        return;

    Gamestate.level = level;

    char str[5] = { 0 };
    color32bpp_t white = {.rgb = 0x00FFFFFF};

    srfc_puts(surf, "\b\b\b\b", Level_pos, Score_bar_bg_clr, SCORE_TEXT_BPP);
    srfc_puts(surf, itoa(level, str, 16), Level_pos, white, SCORE_TEXT_BPP);
}

static void set_lines(uint32_t lines, srfc_t* surf)
{
    if (Gamestate.lines == lines)
        return;

    Gamestate.lines = lines;

    char str[5] = { 0 };
    color32bpp_t white = {.rgb = 0x00FFFFFF};

    srfc_puts(surf, "\b\b\b\b", Lines_pos, Score_bar_bg_clr, SCORE_TEXT_BPP);
    srfc_puts(surf, itoa(lines, str, 16), Lines_pos, white, SCORE_TEXT_BPP);
}

static void draw_hold(srfc_t* surf)
{

}

static void run_loop(void)
{
    
}
