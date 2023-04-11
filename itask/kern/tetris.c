#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>
#include <inc/string.h>

#include <kern/random.h>
#include <kern/tetris.h>
#include <kern/tsc.h>
#include <kern/console.h>

static pair16_t Field_tl;

static pair16_t Score_pos;
static pair16_t Level_pos;
static pair16_t Lines_pos;

static pair16_t Next_pos_tl;
static pair16_t Hold_pos_tl;

static struct Tetris_gamestate
{
    bool game_is_on;
    bool gamestate_updated;

    struct Field_elem
    {
        bool present;
        color32bpp_t color;

    } field[FIELD_HEIGHT][FIELD_WIDTH];

    unsigned score;
    bool score_updated;

    unsigned level;
    bool level_updated;

    unsigned lines;
    bool lines_updated;

    struct Figure_info
    {
        figure_type_t type;
        pair16_t pos_tl;
        unsigned rot;

    } cur_fig;
    bool cur_fig_updated;

    figure_type_t next;
    bool next_updated;

    figure_type_t hold;
    bool hold_updated;

    uint64_t frames_per_cell;

    bool pg_updated;
    bool use_hold;

} Gamestate;


static void set_up_res(void);
static void draw_initial(void);
static void set_up_rand(void);
static void set_up_initial_gamestate(void);

static void draw_field_initial(srfc_t* surf);
static void draw_logo_initial(srfc_t* surf);
static void draw_frame_initial(srfc_t* surf);

static void draw_pg_field_elems(srfc_t* surf);
static void draw_field_elem(srfc_t* surf, pair16_t bl, pair16_t tr, color32bpp_t clr);
static void draw_empty_box(srfc_t* surf, pair16_t bl, pair16_t tr);

static void fill_empty_figure(srfc_t* surf, pair16_t pos_tl);
static void draw_figure(srfc_t* surf, pair16_t pos_tl, const figure_t* fig);

static void draw_next_initial(srfc_t* surf);
static void draw_hold_initial(srfc_t* surf);
static void draw_stat_initial(srfc_t* surf);

static void draw_next(srfc_t* surf);
static void draw_hold(srfc_t* surf);

static void fill_empty_next(srfc_t* surf);
static void fill_empty_hold(srfc_t* surf);

static void draw_score(srfc_t* surf);
static void draw_level(srfc_t* surf);
static void draw_lines(srfc_t* surf);

static void draw_gameover_msg(srfc_t* surf);

static enum Figure_type get_rand_figure_type(void);
static bool figure_pos_is_allowed(const struct Figure_info* figure_info);

static void run_loop(void);

static void model_update_on_key(int key);
static void model_update_on_timer(void);

static enum Tetris_ctrl_key int2ctrl_key(int key);
static bool model_is_line_filled(unsigned line_no);

static void model_process_hold(void);
static void model_update_level(void);
static void model_process_key(enum Tetris_ctrl_key key);
static void model_process_one_cycle(void);
static void model_move_lines(unsigned line_no);
static void model_remove_filled_lines(void);
static void model_emplace_cur_fig(void);
static void model_new_fig(void);
static void model_update_field(uint64_t frame_ct);
static void model_update_score(unsigned removed_lines);

static void draw_view(void); 

static void raise_all_updates(void);
static void acknowledge_all_updates(void);

int tetris(void)
{
    bool is_gpu_ready = gpu_ready();
    if (is_gpu_ready == false)
        return -E_DEV_RT;

    set_up_rand();

    set_up_res();
    draw_initial();
    set_up_initial_gamestate();

    run_loop();

    return 0;
}

static void set_up_initial_gamestate(void)
{
    Gamestate.game_is_on = true;

    Gamestate.level = INITIAL_LEVEL;
    Gamestate.score = 0;
    Gamestate.lines = 0;

    srfc_t surf = { 0 };

    int err = gpu_request_surface(&surf, REQUEST_CUR);
    if (err < 0) panic("gpu_request_surface(): %i \n", err);

    raise_all_updates();

    Gamestate.use_hold = false;
    Gamestate.hold = NONE;
    draw_hold(&surf);

    Gamestate.next = get_rand_figure_type();
    draw_next(&surf);

    Gamestate.cur_fig.type = NONE;

    Gamestate.frames_per_cell = (uint64_t) (1 / (Tetris_speed_table[INITIAL_LEVEL]));

    for (unsigned y = 0; y < FIELD_HEIGHT; y++)
    {
        for (unsigned x = 0; x < FIELD_WIDTH; x++)
        {
            Gamestate.field[y][x].present = false;
        }
    }

    err = gpu_submit_surface(&surf);
    if (err < 0) panic("gpu_submit_surface(): %i \n", err);

    err = gpu_page_flip();
    if (err < 0) panic("gpu_page_flip(): %i \n", err);
}

static void set_up_res(void)
{
    pair16_t res = gpu_get_display_res();

    if (res.x != 1024U || res.y != 768U)
    {
        int err = gpu_set_display_res((pair16_t){.x = 1024U, .y = 768U});
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

    draw_field_initial(&surf);
    draw_frame_initial(&surf);
    draw_logo_initial(&surf);

    draw_stat_initial(&surf);
    draw_hold_initial(&surf);
    draw_next_initial(&surf);

    err = gpu_submit_surface(&surf);
    if (err < 0) panic("gpu_submit_surface(): %i \n", err);

    err = gpu_page_flip();
    if (err < 0) panic("gpu_page_flip(): %i \n", err);
}

static void draw_field_initial(srfc_t* surf)
{
    pair16_t center = {.x = surf->res.x / 2, .y = surf->res.y / 2};

    pair16_t bl = {.x = center.x - FIELD_BAR_WIDTH / 2, .y = center.y + FIELD_BAR_HEIGHT / 2};
    pair16_t tr = {.x = center.x + FIELD_BAR_WIDTH / 2, .y = center.y - FIELD_BAR_HEIGHT / 2};

    srfc_box_thick_in(surf, bl ,tr, Field_bar_outl_clr, FIELD_BAR_OUTL_THICKNESS);

    Field_tl.x = center.x - FIELD_BAR_WIDTH  / 2 + FIELD_BAR_OUTL_THICKNESS; 
    Field_tl.y = center.y - FIELD_BAR_HEIGHT / 2 + FIELD_BAR_OUTL_THICKNESS;

    draw_pg_field_elems(surf);
}

static void draw_pg_field_elems(srfc_t* surf)
{
    pair16_t bl = {.x = Field_tl.x, .y = Field_tl.y + BLOCK_SIZE};
    pair16_t tr = {.x = Field_tl.x + BLOCK_SIZE, .y = Field_tl.y};

    for (unsigned y = 0; y < FIELD_HEIGHT; y++)
    {
        for (unsigned x = 0; x < FIELD_WIDTH; x++)
        {
            if (Gamestate.field[y][x].present == false)
                draw_empty_box(surf, bl, tr);
            else 
                draw_field_elem(surf, bl, tr, Gamestate.field[y][x].color);
        
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

static void draw_logo_initial(srfc_t* surf)
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

    const char* author = "by Rustamchik (2023)";
    pair16_t author_pos = {.x = surf->res.x - 8 * 2 * strlen(author) - BLOCK_SIZE,
                           .y = surf->res.y - 8 * 2 - BLOCK_SIZE};
    srfc_puts(surf, author, author_pos, White_color, 2);
}

static void draw_frame_initial(srfc_t* surf)
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

static void draw_stat_initial(srfc_t* surf)
{
    pair16_t score_bar_tl = {.x = Field_tl.x + FIELD_BAR_WIDTH + BLOCK_SIZE - FIELD_BAR_OUTL_THICKNESS,
                             .y = Field_tl.y - FIELD_BAR_OUTL_THICKNESS};

    pair16_t score_bar_bl = {.x = score_bar_tl.x, .y = score_bar_tl.y + STAT_BAR_HEIGHT};
    pair16_t score_bar_tr = {.x = score_bar_tl.x + STAT_BAR_WIDTH, .y = score_bar_tl.y};

    srfc_bar(surf, score_bar_bl, score_bar_tr, Stat_bar_bg_clr);
    srfc_box_thick_in(surf, score_bar_bl, score_bar_tr, Stat_bar_outl_clr, STAT_BAR_OUTL_THICKNESS);

    score_bar_bl.x += BLOCK_SIZE / 2;

    const uint8_t ppb = STAT_TEXT_BPP;
    const unsigned string_num = 3;

    const char* string[3] = {"LINES:", "LEVEL:", "SCORE:"};
    pair16_t*   coords[3] = {&Lines_pos, &Level_pos, &Score_pos};

    pair16_t num_pos = {.x = score_bar_bl.x + BLOCK_SIZE * 4 + BLOCK_SIZE / 2,
                        .y = score_bar_bl.y};

    for (unsigned iter = 0; iter < string_num; iter++)
    {
        score_bar_bl.y -= BLOCK_SIZE + BLOCK_SIZE / 2;
        srfc_puts(surf, string[iter], score_bar_bl, White_color, ppb);
    
        num_pos.y -= BLOCK_SIZE + BLOCK_SIZE / 2;
        *(coords[iter]) = num_pos;
        srfc_puts(surf, "0000", num_pos, White_color, ppb);
    }
}

static void draw_score(srfc_t* surf)
{
    char str[5] = { 0 };

    srfc_puts(surf, "\b\b\b\b", Score_pos, Stat_bar_bg_clr, STAT_TEXT_BPP);
    srfc_puts(surf, itoa(Gamestate.score, str, 16), Score_pos, White_color, STAT_TEXT_BPP);
}

static void draw_level(srfc_t* surf)
{
    char str[5] = { 0 };

    srfc_puts(surf, "\b\b\b\b", Level_pos, Stat_bar_bg_clr, STAT_TEXT_BPP);
    srfc_puts(surf, itoa(Gamestate.level, str, 16), Level_pos, White_color, STAT_TEXT_BPP);
}

static void draw_lines(srfc_t* surf)
{
    char str[5] = { 0 };

    srfc_puts(surf, "\b\b\b\b", Lines_pos, Stat_bar_bg_clr, STAT_TEXT_BPP);
    srfc_puts(surf, itoa(Gamestate.lines, str, 16), Lines_pos, White_color, STAT_TEXT_BPP);
}

static void draw_next_initial(srfc_t* surf)
{
    pair16_t next_bar_tl = {.x = Field_tl.x + FIELD_BAR_WIDTH + BLOCK_SIZE - FIELD_BAR_OUTL_THICKNESS,
                            .y = Field_tl.y - FIELD_BAR_OUTL_THICKNESS + STAT_BAR_HEIGHT + BLOCK_SIZE};

    pair16_t next_bar_bl = {.x = next_bar_tl.x, .y = next_bar_tl.y + NEXT_BAR_HEIGHT};
    pair16_t next_bar_tr = {.x = next_bar_tl.x + NEXT_BAR_WIDTH, .y = next_bar_tl.y};

    srfc_bar(surf, next_bar_bl, next_bar_tr, Next_bar_bg_clr);
    srfc_box_thick_in(surf, next_bar_bl, next_bar_tr, Next_bar_outl_clr, NEXT_BAR_OUTL_THICKNESS);

    const char* str = "NEXT:";
    unsigned len = strlen(str);
    uint16_t pixel_len = len * 8 * NEXT_TEXT_BPP;

    pair16_t str_pos = {.x = next_bar_tl.x + (NEXT_BAR_WIDTH - pixel_len) / 2,
                        .y = next_bar_tl.y + 8 * NEXT_TEXT_BPP};

    srfc_puts(surf, str, str_pos, White_color, NEXT_TEXT_BPP);

    Next_pos_tl.x = next_bar_tl.x + (NEXT_BAR_WIDTH - FIGURE_SIZE * BLOCK_SIZE) / 2;
    Next_pos_tl.y = next_bar_tl.y + 7 * BLOCK_SIZE / 4;

    fill_empty_next(surf);
}

static void draw_hold_initial(srfc_t* surf)
{
    pair16_t hold_bar_tl = {.x = Field_tl.x + FIELD_BAR_WIDTH + BLOCK_SIZE - FIELD_BAR_OUTL_THICKNESS,
                            .y = Field_tl.y - FIELD_BAR_OUTL_THICKNESS + STAT_BAR_HEIGHT + NEXT_BAR_HEIGHT +2 * BLOCK_SIZE};

    pair16_t hold_bar_bl = {.x = hold_bar_tl.x, .y = Field_tl.y + FIELD_BAR_HEIGHT - FIELD_BAR_OUTL_THICKNESS};
    pair16_t hold_bar_tr = {.x = hold_bar_tl.x + HOLD_BAR_WIDTH, .y = hold_bar_tl.y};

    srfc_bar(surf, hold_bar_bl, hold_bar_tr, Hold_bar_bg_clr);
    srfc_box_thick_in(surf, hold_bar_bl, hold_bar_tr, Hold_bar_outl_clr, HOLD_BAR_OUTL_THICKNESS);

    const char* str = "HOLD:";
    unsigned len = strlen(str);
    uint16_t pixel_len = len * 8 * HOLD_TEXT_BPP;

    pair16_t str_pos = {.x = hold_bar_tl.x + (HOLD_BAR_WIDTH - pixel_len) / 2,
                        .y = hold_bar_tl.y + 8 * HOLD_TEXT_BPP};

    srfc_puts(surf, str, str_pos, White_color, HOLD_TEXT_BPP);

    Hold_pos_tl.x = hold_bar_tl.x + (HOLD_BAR_WIDTH - FIGURE_SIZE * BLOCK_SIZE) / 2;
    Hold_pos_tl.y = hold_bar_tl.y + 7 * BLOCK_SIZE / 4;

    fill_empty_hold(surf);
}

static void draw_figure(srfc_t* surf, pair16_t pos_tl, const figure_t* fig)
{
    pair16_t bl = {.x = pos_tl.x, .y = pos_tl.y + BLOCK_SIZE};
    pair16_t tr = {.x = pos_tl.x + BLOCK_SIZE, .y = pos_tl.y};

    for (unsigned y = 0; y < FIGURE_SIZE; y++)
    {
        for (unsigned x = 0; x < FIGURE_SIZE; x++)
        {
            if (fig->map[y][x] == 1)
            {
                draw_field_elem(surf, bl, tr, fig->color);
            }
        
            bl.x += BLOCK_SIZE;
            tr.x += BLOCK_SIZE;
        }

        bl.y += BLOCK_SIZE;
        tr.y += BLOCK_SIZE;
    
        bl.x = pos_tl.x;
        tr.x = pos_tl.x + BLOCK_SIZE;
    }
}

static void draw_next(srfc_t* surf)
{
    fill_empty_next(surf);

    if (Gamestate.next != NONE)
        draw_figure(surf, Next_pos_tl, &Figures[Gamestate.next][0]);
}

static void draw_hold(srfc_t* surf)
{
    fill_empty_hold(surf);

    if (Gamestate.hold != NONE)
        draw_figure(surf, Hold_pos_tl, &Figures[Gamestate.hold][0]);
}

static void fill_empty_figure(srfc_t* surf, pair16_t pos_tl)
{
    pair16_t bl = {.x = pos_tl.x, .y = pos_tl.y + BLOCK_SIZE};
    pair16_t tr = {.x = pos_tl.x + BLOCK_SIZE, .y = pos_tl.y};

    for (unsigned y = 0; y < FIGURE_SIZE; y++)
    {
        for (unsigned x = 0; x < FIGURE_SIZE; x++)
        {
            draw_empty_box(surf, bl, tr);
        
            bl.x += BLOCK_SIZE;
            tr.x += BLOCK_SIZE;
        }

        bl.y += BLOCK_SIZE;
        tr.y += BLOCK_SIZE;
    
        bl.x = pos_tl.x;
        tr.x = pos_tl.x + BLOCK_SIZE;
    }
}

static void fill_empty_next(srfc_t* surf)
{
    fill_empty_figure(surf, Next_pos_tl);
}

static void fill_empty_hold(srfc_t* surf)
{
    fill_empty_figure(surf, Hold_pos_tl);
}

static void set_up_rand(void)
{
    uint64_t cpu_freq = get_cpu_frequency(HPET0);
    srand((unsigned) (cpu_freq));
}

static enum Figure_type get_rand_figure_type(void)
{
    return I_BLOCK + (rand() % N_BLOCK_TYPES);
}

static void run_loop(void)
{
    int err = ktimer_start(Timer_timeout_ms, Timer_type);
    if (err < 0) panic("ktimer_start(): %d \n", err);

    while (1)
    {
        int res  = 0;
        int symb = 0;

        do
        {
            res = ktimer_poll();
            if (res < 0) panic("ktimer_poll(): %d \n", res);

            symb = cons_getc();
            if (symb != 0)
                model_update_on_key(symb);

            if ((symb == 'r' || symb == 'R') && Gamestate.game_is_on == false)
            {
                draw_initial();
                set_up_initial_gamestate();
                goto reset;
            }

        } while (res != 1);

        model_update_on_timer();
        draw_view();

    reset:

        err = ktimer_reset(Timer_timeout_ms, Timer_type);
        if (err < 0) panic("ktimer_reset(): %d \n", err);
    }
}

static void model_update_on_key(int key)
{
    if (Gamestate.game_is_on == false)
        return;

    model_process_key(int2ctrl_key(key));
}

static void model_update_on_timer(void)
{
    if (Gamestate.game_is_on == false)
        return;

    static uint64_t frame_ct  = 0;

    if (Gamestate.lines_updated == true
     && Gamestate.cur_fig.type == NONE)
    {
        model_update_level();
    }

    if (Gamestate.cur_fig.type == NONE)
        model_new_fig();
    else 
        model_update_field(frame_ct);

    frame_ct += 1;
    return;
}

static void model_new_fig(void)
{
    Gamestate.cur_fig.rot    = 0;
    Gamestate.cur_fig.pos_tl = (pair16_t) {.x = FIELD_WIDTH / 2, .y = 0};
    
    if (Gamestate.use_hold == false)
    {
        Gamestate.cur_fig.type = Gamestate.next;
        Gamestate.next = get_rand_figure_type();
        
        Gamestate.next_updated = true;
    }
    else 
    {
        Gamestate.cur_fig.type = Gamestate.hold;
        Gamestate.hold = NONE;

        Gamestate.use_hold = false;
        Gamestate.hold_updated = true;
    }

    if (figure_pos_is_allowed(&Gamestate.cur_fig) == false)
    {
        Gamestate.game_is_on = false;
        Gamestate.gamestate_updated = true;
    }

    Gamestate.cur_fig_updated = true;
}

static void model_update_field(uint64_t frame_ct)
{
    unsigned num_cycles = 1U;

    if (Gamestate.frames_per_cell > 1)
    {
        if ((frame_ct % Gamestate.frames_per_cell) != 0)
            return;
    }
    else 
        num_cycles = (unsigned) Tetris_speed_table[Gamestate.level];

    for (unsigned iter = 0; iter < num_cycles; iter++)
        model_process_one_cycle();
}

static bool figure_pos_is_allowed(const struct Figure_info* figure_info)
{
    const char (*map)[FIGURE_SIZE] = Figures[figure_info->type][figure_info->rot].map;

    for (unsigned y = 0; y < FIGURE_SIZE; y++)
    {
        for (unsigned x = 0; x < FIGURE_SIZE; x++)
        {
            pair16_t cur_coord = { .x = figure_info->pos_tl.x + x,
                                   .y = figure_info->pos_tl.y + y };

            if (map[y][x])
            {
                if (cur_coord.x < 0 || cur_coord.y < 0
                 || cur_coord.x >= FIELD_WIDTH || cur_coord.y >= FIELD_HEIGHT)
                {
                    return false;
                }

                if (Gamestate.field[cur_coord.y][cur_coord.x].present == true)
                    return false;
            }
        }
    }

    return true;
}

static void model_process_one_cycle(void)
{
    struct Figure_info next_pos_fig = {.type = Gamestate.cur_fig.type,
                                       .rot  = Gamestate.cur_fig.rot,
                                       .pos_tl = {.x = Gamestate.cur_fig.pos_tl.x,
                                                  .y = Gamestate.cur_fig.pos_tl.y + 1}};

    if (figure_pos_is_allowed(&next_pos_fig) == true)
        Gamestate.cur_fig = next_pos_fig;
    else 
        model_emplace_cur_fig();

    model_remove_filled_lines();

    Gamestate.cur_fig_updated = true;
    Gamestate.pg_updated = true;
}

static bool model_is_line_filled(unsigned line_no)
{
    for (unsigned iter = 0; iter < FIELD_WIDTH; iter++)
    {
        if (Gamestate.field[line_no][iter].present == false)
            return false;
    }

    return true;
}

static void model_move_lines(unsigned line_no)
{
    for (unsigned y = line_no; y > 0; y--)
    {
        for (unsigned x = 0; x < FIELD_WIDTH; x++)
        {
            Gamestate.field[y][x].present = Gamestate.field[y - 1][x].present;
            Gamestate.field[y][x].color   = Gamestate.field[y - 1][x].color;
        }
    }

    for (unsigned x = 0; x < FIELD_WIDTH; x++)
        Gamestate.field[0][x].present = false;
}

static void model_remove_filled_lines(void)
{
    unsigned removed_lines = 0;

    for (unsigned line_no = FIELD_HEIGHT - 1; line_no > 0;)
    {
        if (model_is_line_filled(line_no) == true)
        {
            model_move_lines(line_no);

            removed_lines   += 1;
            Gamestate.lines += 1;

            Gamestate.lines_updated = true;
        }
        else 
            line_no--;
    }

    if (model_is_line_filled(0) == true)
    {
        for (unsigned x = 0; x < FIELD_WIDTH; x++)
        {
            Gamestate.field[0][x].present = false;
        }
    }

    if (removed_lines > 0)
        model_update_score(removed_lines);
}

static void model_update_score(unsigned removed_lines)
{
    unsigned points = 0;

    switch (removed_lines)
    {
        case 0:  points =    0U; break;
        case 1:  points =   40U; break;
        case 2:  points =  100U; break;
        case 3:  points =  300U; break;
        case 4:  
        default: points = 1200U; break;
    }

    Gamestate.score += points * (Gamestate.level + 1);
    Gamestate.score_updated = true;
}

static void model_emplace_cur_fig(void)
{
    struct Figure_info cur_fig = Gamestate.cur_fig;
    color32bpp_t cur_fig_clr = Figures[cur_fig.type][cur_fig.rot].color;

    const char (*map)[FIGURE_SIZE] = Figures[cur_fig.type][cur_fig.rot].map;

    for (unsigned y = 0; y < FIGURE_SIZE; y++)
    {
        for (unsigned x = 0; x < FIGURE_SIZE; x++)
        {
            pair16_t cur_coord = { .x = cur_fig.pos_tl.x + x,
                                   .y = cur_fig.pos_tl.y + y };

            if (map[y][x])
            {
                Gamestate.field[cur_coord.y][cur_coord.x].present = true;
                Gamestate.field[cur_coord.y][cur_coord.x].color = cur_fig_clr;
            }
        }
    }

    Gamestate.cur_fig.type = NONE;
}

static enum Tetris_ctrl_key int2ctrl_key(int key)
{
    switch (key)
    {
        case 'a': 
        case 'A': return LEFT;

        case 's': 
        case 'S': return DOWN;

        case 'd': 
        case 'D': return RIGHT;

        case 'w': 
        case 'W': return ROT;

        case 'h':
        case 'H': return HOLD;

        default: break;
    }

    return NO_ACTION;
}


static void model_process_key(enum Tetris_ctrl_key key)
{
    if (Gamestate.cur_fig.type == NONE || key == NO_ACTION)
        return;

    if (key == HOLD)
    {
        model_process_hold();
        return;
    }
    else 
        Gamestate.use_hold = false;

    struct Figure_info next_pos_fig = Gamestate.cur_fig;

    switch (key)
    {
        case LEFT:  next_pos_fig.pos_tl.x -= 1; break;
        case DOWN:  next_pos_fig.pos_tl.y += 1; break;
        case RIGHT: next_pos_fig.pos_tl.x += 1; break;
        case ROT:
        {
            next_pos_fig.rot = (next_pos_fig.rot + 1) % N_BLOCK_ROT;
            break;
        }
        default: break;
    }

    if (figure_pos_is_allowed(&next_pos_fig) == true)
    {
        Gamestate.cur_fig = next_pos_fig;

        Gamestate.cur_fig_updated = true;
        Gamestate.pg_updated = true;
    }
}

static void model_process_hold(void)
{
    if (Gamestate.hold == NONE && Gamestate.cur_fig.type != NONE)
    {
        Gamestate.hold = Gamestate.cur_fig.type;
        Gamestate.cur_fig.type = NONE;

        Gamestate.cur_fig_updated = true;
        Gamestate.hold_updated = true;
    }
    else 
        Gamestate.use_hold = true;
}

static void model_update_level(void)
{
    if (Gamestate.lines >= Gamestate.level * LINES_PER_LEVEL)
    {
        if (Gamestate.level < TETRIS_LEVELS_NUM)
        {
            Gamestate.level += 1;
            Gamestate.frames_per_cell = (uint64_t) (1 / Tetris_speed_table[Gamestate.level]);

            Gamestate.level_updated = true;
        }
    }
}

static void draw_view(void)
{
    srfc_t surf = { 0 };

    int err = gpu_request_surface(&surf, REQUEST_CUR);
    if (err < 0) panic("gpu_request_surface(): %i \n", err);

    if (Gamestate.score_updated)
        draw_score(&surf);

    if (Gamestate.level_updated)
        draw_level(&surf);

    if (Gamestate.lines_updated)
        draw_lines(&surf);

    if (Gamestate.next_updated)
        draw_next(&surf);
    
    if (Gamestate.hold_updated)    
        draw_hold(&surf);

    if (Gamestate.cur_fig_updated)
    {
        draw_pg_field_elems(&surf);

        pair16_t cur_fig_tl = {.x = Field_tl.x + Gamestate.cur_fig.pos_tl.x * BLOCK_SIZE,
                            .y = Field_tl.y + Gamestate.cur_fig.pos_tl.y * BLOCK_SIZE};
        figure_t fig = Figures[Gamestate.cur_fig.type][Gamestate.cur_fig.rot];
        draw_figure(&surf, cur_fig_tl, &fig);
    }

    if (Gamestate.gamestate_updated && (Gamestate.game_is_on == false))
        draw_gameover_msg(&surf);

    err = gpu_submit_surface(&surf);
    if (err < 0) panic("gpu_submit_surface(): %i \n", err);

    err = gpu_page_flip();
    if (err < 0) panic("gpu_page_flip(): %i \n", err);

    acknowledge_all_updates();
}

static void draw_gameover_msg(srfc_t* surf)
{
    uint8_t bpp = 10;

    pair16_t tl = {.x = (surf->res.x - 10 * 8 * bpp) / 2,
                   .y = (surf->res.y -  1 * 8 * bpp) / 2};

    pair16_t bl = {.x = tl.x,
                   .y = tl.y +  1 * 8 * bpp};

    pair16_t tr = {.x = tl.x + 10 * 8 * bpp,
                   .y = tl.y};

    srfc_bar(surf, bl, tr, Black_color);
    srfc_puts(surf, "GAME OVER!", tl, Red_color, bpp);
}

static void raise_all_updates(void)
{
    Gamestate.cur_fig_updated = true;

    Gamestate.score_updated = true;
    Gamestate.level_updated = true;
    Gamestate.lines_updated = true;

    Gamestate.next_updated = true;
    Gamestate.hold_updated = true;

    Gamestate.cur_fig_updated = true;
    Gamestate.pg_updated = true;

    Gamestate.gamestate_updated = true;
}

static void acknowledge_all_updates(void)
{
    Gamestate.cur_fig_updated = false;

    Gamestate.score_updated = false;
    Gamestate.level_updated = false;
    Gamestate.lines_updated = false;

    Gamestate.next_updated = false;
    Gamestate.hold_updated = false;

    Gamestate.cur_fig_updated = false;
    Gamestate.pg_updated = false;

    Gamestate.gamestate_updated = false;
}