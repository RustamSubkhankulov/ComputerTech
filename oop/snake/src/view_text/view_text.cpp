#include <iostream>
#include <termios.h>
#include <sys/ioctl.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cassert>

//---------------------------------------------------------

#include "../../include/view_text/view_text.hpp"

//=========================================================

static const size_t Top_padding = 3;
static const size_t Btm_padding = 3;
static const size_t Lft_padding = 1;
static const size_t Rgt_padding = 1;

static const int CSI_fg_color_code = 30;
static const int CSI_bg_color_code = 40; 

static const int CSI_bold_on      = 1;
static const int CSI_blink_on     = 5;
static const int CSI_underline_on = 21;

static const int CSI_bold_off      = 22;
static const int CSI_blink_off     = 25;
static const int CSI_underline_off = 24;

//---------------------------------------------------------

static bool Wnsz_renew_needed = true;
// static global variable since static functions cannot be declared as friend w/o forward declaration

//---------------------------------------------------------

static const char Rabbit_symb = 'R';

static const char Snake_symb      = 'S';
// static const char Snake_symb_head = 'H';

//=========================================================

static void sighandler(int signum);

//=========================================================

void View_text::reset_attr()
{
    printf("\e[0m");
    fflush(stdout);
}

//---------------------------------------------------------

void View_text::set_attr(enum View_text::Color fg, enum View_text::Color bg, 
                                      bool bold, bool blink, bool underline)
{
    int fg_csi_code = (int) fg + CSI_fg_color_code; 
    int bg_csi_code = (int) bg + CSI_bg_color_code;

    printf("\e[%d;%dm", fg_csi_code, bg_csi_code);
    printf("\e[%d;%d;%dm", (bold)? CSI_bold_on: CSI_bold_off,
                           (blink)? CSI_blink_on : CSI_blink_off,
                           (underline)? CSI_underline_on : CSI_underline_off);
    fflush(stdout);
}

//---------------------------------------------------------

void View_text::turn_off_carriage()
{
    printf("\e[?25l");
    fflush(stdout);
}

//---------------------------------------------------------

void View_text::turn_on_carriage()
{
    printf("\e[?25h");
    fflush(stdout);
}

//---------------------------------------------------------

void View_text::set_sighandler()
{
    struct sigaction newact = {.__sigaction_handler = sighandler,
                               .sa_mask    = 0,
                               .sa_flags   = 0};
    int err = sigaction(SIGWINCH, &newact, &oldact_);
    if (err != 0)
    {
        fprintf(stderr, "sigaction() failed: %s \n", strerror(errno));
        assert(false);
    }
}

//---------------------------------------------------------

void View_text::restore_sighandler()
{
    int err = sigaction(SIGWINCH, &oldact_, NULL);
    if (err != 0)
    {
        fprintf(stderr, "sigaction() failed: %s \n", strerror(errno));
        assert(false);
    }
}

//---------------------------------------------------------

static void sighandler(int signum)
{
    View_text* self = dynamic_cast<View_text*>(View::get_view());
    Wnsz_renew_needed = true;
}

//---------------------------------------------------------

void View_text::draw()
{    
    cls();
    draw_frame();

    Model* model = get_model();
    if (model != nullptr);
    {
        model->update(get_winsize());
        draw_rabbits(model);
        draw_snakes(model);
    }

    pause();
}

//---------------------------------------------------------

void View_text::draw_rabbits(Model* model)
{
    for (const Rabbit& rabbit : model->rabbits)
    {
        putxy(Rabbit_symb, rabbit.get_coords());
    }
}

//---------------------------------------------------------

void View_text::draw_snakes(Model* model)
{
    for (const Snake& snake : model->snakes)
    {
        Coords_list coords_list = snake.get_coords_list();

        for (const Coords& coords : coords_list)
        {
            putxy(Snake_symb, coords);
        }
    }
}

//---------------------------------------------------------

Vector View_text::get_winsize() const
{
    if (Wnsz_renew_needed == true)
    {
        struct winsize wnsz{};
        int err = ioctl(0, TIOCGWINSZ, &wnsz);
        if (err < 0)
        {
            fprintf(stderr, "iocctl() failed: %s \n", strerror(errno));
            assert(false);
        }

        wnsz_ = Vector{(ssize_t) wnsz.ws_col, (ssize_t) wnsz.ws_row};
        Wnsz_renew_needed = false;
    }    

    return wnsz_;
}

//---------------------------------------------------------

void View_text::putxy(const char sym, const Vector& coord)
{
    printf("\e[%lu;%luH", (size_t) coord.y(), (size_t) coord.x());
    printf("%c", sym);
    fflush(stdout);
}

//---------------------------------------------------------

void View_text::printxy(const std::string& str, const Vector& coord)
{
    printf("\e[%lu;%luH", (size_t) coord.y(), (size_t) coord.x());
    printf("%s", str.c_str());
    fflush(stdout);   
}

//---------------------------------------------------------

void View_text::hline(const size_t y, const size_t x0, const size_t x1, const char sym)
{
    for (size_t x = x0; x <= x1; x++)
    {
        putxy(sym, Vector{(ssize_t) x, (ssize_t) y});
    }
}

//---------------------------------------------------------

void View_text::vline(const size_t x, const size_t y0, const size_t y1, const char sym)
{
    for (size_t y = y0; y <= y1; y++)
    {
        putxy(sym, Vector{(ssize_t) x, (ssize_t) y});
    }
}

//---------------------------------------------------------

void View_text::cls()
{
    printf("\e[2J");
    fflush(stdout);
}

//---------------------------------------------------------

void View_text::draw_frame()
{
    Vector wnsz = get_winsize();
    size_t cols = (size_t) wnsz.x();
    size_t rows = (size_t) wnsz.y();

    set_attr(MAGENTA, BLACK, true, false, false);
    hline(1, 1, cols, '=');

    // hline(2, 1, cols, ' ');
    int fg_col = RED;
    for (size_t ncol = 1; ncol <= cols; ncol++)
    {
        if (fg_col == BLACK)
            fg_col++;
        if (fg_col == DEF)
            fg_col = RED;
        
        set_attr((Color) fg_col, BLACK, false, false, false);
        putxy('~', Vector{(ssize_t) ncol, 2});
    
        fg_col++;
    }

    set_attr(MAGENTA, BLACK, true, false, false);
    hline(3, 1, cols, '=');

    set_attr(WHITE, WHITE, false, false, false);
    for (size_t nrow = 4; nrow < rows - 2; nrow++)
    {
        hline(nrow, 1, cols, ' ');
    }

    set_attr(MAGENTA, BLACK, true, false, false);
    hline(rows - 2, 1, cols, '=');
    hline(rows - 1, 1, cols, ' ');
    hline(rows, 1, cols -1, '=');

    vline(1, 1, rows, '|');    
    vline(cols, 1, rows - 1, '|');

    set_attr(GREEN, BLACK, true, false, true);
    std::string title{"SNAKE GAME (TM)"};
    size_t ncol = (cols - title.size()) / 2;
    printxy(title, Vector{(ssize_t) ncol, 2});

    set_attr(CYAN, BLACK, false, false, false);
    std::string author_str{"by Rustamchik"};
    ncol = cols - author_str.size() - Rgt_padding;
    printxy(author_str, Vector{(ssize_t) ncol, (ssize_t) rows - 1});
}
