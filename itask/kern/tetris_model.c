#include <inc/assert.h>
#include <inc/error.h>
#include <inc/x86.h>
#include <inc/string.h>

#include <kern/random.h>
#include <kern/tetris_model.h>
#include <kern/tetris_view.h>
#include <kern/tsc.h>
#include <kern/console.h>

struct Tetris_gamestate Gamestate = { 0 };

static void set_up_rand(void);
static void set_up_initial_gamestate(void);

static enum Figure_type get_rand_figure_type(void);
static bool figure_pos_is_allowed(const struct Figure_info* figure_info);

static void run_loop(void);
void draw_view(void);

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

static void raise_all_updates(void);
static void acknowledge_all_updates(void);

int tetris(void)
{
    bool is_gpu_ready = gpu_ready();
    if (is_gpu_ready == false)
        return -E_DEV_RT;

    set_up_rand();

    set_up_res();
    draw_initial(&Gamestate);
    set_up_initial_gamestate();

    run_loop();

    cleanup_display();
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
    draw_hold(&surf, Gamestate.hold);

    Gamestate.next = get_rand_figure_type();
    draw_next(&surf, Gamestate.next);

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

            if (symb == 'r' || symb == 'R')
            {
                draw_initial(&Gamestate);
                set_up_initial_gamestate();
                goto reset;
            }

            if (symb == 'q' || symb == 'Q')
                return;

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
    {
        model_update_score(removed_lines);
        model_update_level();
    }

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
        case HOLD:  model_process_hold(); return;
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

void draw_view(void)
{
    srfc_t surf = { 0 };

    int err = gpu_request_surface(&surf, REQUEST_CUR);
    if (err < 0) panic("gpu_request_surface(): %i \n", err);

    if (Gamestate.score_updated)
        draw_score(&surf, Gamestate.score);

    if (Gamestate.level_updated)
        draw_level(&surf, Gamestate.level);

    if (Gamestate.lines_updated)
        draw_lines(&surf, Gamestate.lines);

    if (Gamestate.next_updated)
        draw_next(&surf, Gamestate.next);
    
    if (Gamestate.hold_updated)    
        draw_hold(&surf, Gamestate.hold);

    if (Gamestate.cur_fig_updated)
    {
        draw_pg_field_elems(&surf, Gamestate.field);
        draw_cur_fig(&surf, &(Gamestate.cur_fig));
    }

    if (Gamestate.gamestate_updated && (Gamestate.game_is_on == false))
        draw_gameover_msg(&surf);

    err = gpu_submit_surface(&surf);
    if (err < 0) panic("gpu_submit_surface(): %i \n", err);

    err = gpu_page_flip();
    if (err < 0) panic("gpu_page_flip(): %i \n", err);

    acknowledge_all_updates();
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