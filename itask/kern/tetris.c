#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>
#include <inc/string.h>

#include <kern/tetris.h>

static pair16_t Field_tl;

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

static void draw_initial();

static void draw_field(srfc_t* surf);
static void draw_logo(srfc_t* surf);

static void draw_field_elems(srfc_t* surf);
static void draw_field_elem(srfc_t* surf, pair16_t bl, pair16_t tr, color32bpp_t clr);
static void draw_empty_box(srfc_t* surf, pair16_t bl, pair16_t tr);

static void draw_next(srfc_t* surf);
static void draw_hold(srfc_t* surf);
static void draw_score(srfc_t* surf);

static void run_loop(void);

int tetris(void)
{
    bool is_gpu_ready = gpu_ready();
    if (is_gpu_ready == false)
        return -E_DEV_RT;

    draw_initial();

    Gamestate.level = 1;
    Gamestate.score = 0;
    Gamestate.lines = 0;

    run_loop();

    return 0;
}

static void draw_initial()
{
    srfc_t surf = { 0 };

    int err = gpu_request_surface(&surf, 0);
    if (err < 0) panic("gpu_request_surface(): %i \n", err);

    srfc_clear(&surf);
    srfc_fill(&surf, Bg_clr);

    draw_field(&surf);
    draw_logo(&surf);

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
    srfc_bar(surf, bl, tr, clr);
    color32bpp_t outl_clr = {.rgb = (uint32_t)((float) clr.rgb * 0.7)};
    srfc_box_thick_in(surf, bl ,tr, outl_clr, BLOCK_OUTL_THICKNESS);
}

static void draw_empty_box(srfc_t* surf, pair16_t bl, pair16_t tr)
{
    srfc_bar(surf, bl, tr, Empty_box_clr);
    srfc_box_thick_in(surf, bl ,tr, Empty_box_outl_clr, BLOCK_OUTL_THICKNESS);
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
                              Figures[J_BLOCK][0].color,
                              Figures[T_BLOCK][0].color};

    pair16_t sym_pos = {.x = Field_tl.x / 2 - title_len * pixel_per_sym / 2, 
                        .y = surf->res.y / 2 - pixel_per_sym};

    const uint16_t outl_thickness = 4;

    uint16_t mid_y = sym_pos.y + pixel_per_sym / 2;
    uint16_t mid_x = sym_pos.x + title_len * pixel_per_sym / 2;

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

    color32bpp_t yellow = {.rgb = 0x00EEFC5E};
    pair16_t num_pos = {.x = p1.x - 40, .y = p2.y + 20};
    srfc_puts(surf, "100", num_pos, yellow, 8);

    // pair16_t author_pos = {.x = surf->res.x - }
}

static void draw_next(srfc_t* surf)
{

}

static void draw_score(srfc_t* surf)
{

}

static void draw_hold(srfc_t* surf)
{

}

static void run_loop(void)
{
    
}
