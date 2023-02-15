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

//=========================================================

void View_text::draw()
{
    cls();
    draw_frame();
    getchar();
}

//---------------------------------------------------------

Vector View_text::get_winsize() const
{
    struct winsize wnsz{};
    int err = ioctl(0, TIOCGWINSZ, &wnsz);
    if (err < 0)
    {
        fprintf(stderr, "iocctl() failed: %s \n", strerror(errno));
        assert(false);
    }

    return Vector{(ssize_t) wnsz.ws_col, (ssize_t) wnsz.ws_row};
}

//---------------------------------------------------------

void View_text::putxy(const char sym, const Vector& coord)
{
    printf("\e[%lu,%luH", (size_t) coord.y(), (size_t) coord.x());
    printf("%c", sym);
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
    printf("\e[H\e[J");
    fflush(stdout);
}

//---------------------------------------------------------

void View_text::draw_frame()
{
    Vector wnsz = get_winsize();
    size_t cols = (size_t) wnsz.x();
    size_t rows = (size_t) wnsz.y();

    hline(1, 1, cols, '=');

    // hline(3, 1, cols, '=');
    // hline(rows - 2, 1, cols, '=');
    // hline(rows, 1, cols -1, '=');

    // vline(1, 1, rows, '|');
    // vline(cols, 1, rows - 1, '|');
}
