#include <iostream>
#include <algorithm>
#include <iterator>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <climits>
#include <poll.h>
#include <stdexcept>

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

static bool Wn_resized = true;

//---------------------------------------------------------

static const char Rabbit_symb = 'R';

static const char Snake_symb = 'S';

//=========================================================

static void sighandler(int signum);

//=========================================================

void View_text::termios_change_conf()
{
    int err = tcgetattr(STDIN_FILENO, &(this->termis_attr));
    if (err != 0)
    {
        fprintf(stderr, "tcgetattr() failed: %s \n", strerror(errno));
        throw std::runtime_error{"tcgetattr() failed"};
    }

    struct termios attr_raw = this->termis_attr;
    cfmakeraw(&attr_raw);

    err = tcsetattr(STDIN_FILENO, TCSANOW, &attr_raw);
    if (err != 0)
    {
        fprintf(stderr, "tcsetattr() failed: %s \n", strerror(errno));
        throw std::runtime_error{"tcsetattr() failed"};
    }
}

//---------------------------------------------------------

void View_text::termios_restore_conf()
{
    int err = tcsetattr(STDIN_FILENO, TCSANOW, &(this->termis_attr));
    if (err != 0)
    {
        fprintf(stderr, "tcsetattr() failed: %s \n", strerror(errno));
        throw std::runtime_error{"tcsetattr() failed"};
    }
}

//---------------------------------------------------------

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

void View_text::turn_off_carriage() const
{
    printf("\e[?25l");
    fflush(stdout);
}

//---------------------------------------------------------

void View_text::turn_on_carriage() const
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
        throw std::runtime_error{"sigaction() failed"};
    }
}

//---------------------------------------------------------

void View_text::restore_sighandler() const
{
    int err = sigaction(SIGWINCH, &oldact_, NULL);
    if (err != 0)
    {
        fprintf(stderr, "sigaction() failed: %s \n", strerror(errno));
        throw std::runtime_error{"sigaction() failed"};
    }
}

//---------------------------------------------------------

static void sighandler(int signum)
{
    View_text* self = dynamic_cast<View_text*>(View::get_view());
    Wnsz_renew_needed = true;
    Wn_resized = true;
}

//---------------------------------------------------------

int View_text::get_poll_timeout()
{
    if (subs_on_timer.size() == 0)
        return -1;

    int min = INT_MAX;

    for (const auto& sub : subs_on_timer)
    {
        if ((int) sub.first < min)
            min = (int) sub.first;
    }

    return min;
}

//---------------------------------------------------------

timeval View_text::get_cur_time() const
{
    struct timeval cur = { 0 };
    int err = gettimeofday(&cur, NULL);
    if (err != 0)
    {
        fprintf(stderr, "gettimeofday() failed: %s \n", strerror(errno));
        throw std::runtime_error{"gettimeofday() failed"};
    }

    return cur;
}

//---------------------------------------------------------

void View_text::run_loop()
{    

    while (1)
    {
        get_winsize(); // renew if there was resize 
                       // we need to realloc shots' arrays

        timeval start = get_cur_time();

        int timeout = get_poll_timeout();
        poll_events(timeout);

        timeval end = get_cur_time();
        timeval elapsed = { 0 };
        timersub(&end, &start, &elapsed);
        
        serve_subs_on_timer(elapsed);
        
        Model* model = get_model();
        assert(model != nullptr);

        bool model_updated = model->model_is_updated();

        if (model_updated || Wn_resized)
        {
            if (model_updated)
                model->model_aknowledge_update();
            
            draw_frame();

            if (model->game_is_ended())
            {
                draw_lose_msg();
            }
            else 
            {
                draw_rabbits(model);
                draw_snakes(model);
            }
        }

        if (exit == true)
            return;
    }
}

//---------------------------------------------------------

struct Sub_remove_predicate
{
    bool operator() (on_timer_callback callback)
    {
        return (callback.first == 0);
    }
};

void View_text::serve_subs_on_timer(const timeval& elapsed)
{
    unsigned elapsed_ms = elapsed.tv_sec * 1000 + elapsed.tv_usec / 1000;

    size_t count = subs_on_timer.size();

    for (auto& sub : subs_on_timer)
    {
        if (count == 0)
            break;

        if (elapsed_ms < sub.first)
            sub.first -= elapsed_ms;
        else 
        {
            sub.first = 0;
            sub.second();
        }

        count -= 1;
    }

    auto remove = std::find_if(subs_on_timer.begin(), subs_on_timer.end(), Sub_remove_predicate());

    while ((remove) != subs_on_timer.end())
    {
        subs_on_timer.erase(remove);
        remove = std::find_if(subs_on_timer.begin(), subs_on_timer.end(), Sub_remove_predicate());
    }
}

//---------------------------------------------------------

void View_text::poll_events(int timeout)
{
    poll_on_key(timeout);
}

//---------------------------------------------------------

void View_text::poll_on_key(int timeout)
{
    struct pollfd req = {.fd = STDIN_FILENO, .events = POLLIN, .revents = 0 };

    int err = 0;
    errno = 0;

    do 
    {
        err = poll(&req, 1, timeout);

    } while (err == -1 && errno == EINTR);

    if (err == -1)
    {
        fprintf(stderr, "poll() failed: %s \n", strerror(errno));
        throw std::runtime_error{"poll() failed"};
    }

    if (req.revents & POLL_IN)
    {
        char sym = 0;
        err = read(STDIN_FILENO, &sym, 1);
        if (err == -1)
        {
            fprintf(stderr, "read() failed: %s \n", strerror(errno));
            throw std::runtime_error{"read() failed"};
        }

        for (auto func : subs_on_key)
        {
            func((int) sym);
        }

        if (sym == 'q')
        {
            exit = true;
        }
    }
}

//---------------------------------------------------------

void View_text::draw_rabbits(Model* model)
{
    Vector offset{Lft_padding + 1, Top_padding + 1};
    set_attr(BLACK, WHITE, 1, 0, 0);

    for (const Rabbit& rabbit : model->rabbits)
    {
        putxy(Rabbit_symb, rabbit.get_coords() + offset);
    }
}

//---------------------------------------------------------

void View_text::draw_snakes(Model* model)
{
    Vector offset{Lft_padding + 1, Top_padding + 1};
    set_attr(BLACK, GREEN, 1, 0, 0);

    for (const Snake& snake : model->snakes)
    {
        Coords_list coords_list = snake.get_coords_list();

        for (const Coords& coords : coords_list)
        {
            putxy(Snake_symb, coords + offset);
        }
    }
}

//---------------------------------------------------------

Vector View_text::get_winsize_real() const
{
    if (Wnsz_renew_needed == true)
    {
        struct winsize wnsz{};
        int err = ioctl(0, TIOCGWINSZ, &wnsz);
        if (err < 0)
        {
            fprintf(stderr, "iocctl() failed: %s \n", strerror(errno));
            throw std::runtime_error{"iocctl() failed"};
        }

        wnsz_ = Vector{(ssize_t) wnsz.ws_col, (ssize_t) wnsz.ws_row};
        Wnsz_renew_needed = false;
    }    

    return wnsz_;
}

//---------------------------------------------------------

Vector View_text::get_winsize() const
{
    Vector wnsz = get_winsize_real();
    return wnsz - Vector{Lft_padding + Rgt_padding, Top_padding + Btm_padding};
}

//---------------------------------------------------------

void View_text::set_location(const Vector& coord)
{
    printf("\e[%lu;%luH", (size_t) coord.y(), (size_t) coord.x());
    fflush(stdout);
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
    Vector wnsz = get_winsize_real();
    size_t cols = (size_t) wnsz.x();
    size_t rows = (size_t) wnsz.y();

    set_attr(MAGENTA, BLACK, true, false, false);
    hline(1, 1, cols, '=');

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

    set_attr(BROWN, BROWN, false, false, false);
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

    draw_results((ssize_t) (Lft_padding + 1), (ssize_t) (rows - 1));
}

//---------------------------------------------------------

void View_text::draw_results(size_t ncol, size_t nrow)
{    
    Model* model = get_model();
    assert(model != nullptr);

    std::string str{"RESULTS: "};

    unsigned ct = 0;

    for (const auto& snake : model->snakes)
    {
        str += "#";
        str += std::to_string(ct);

        str += " SCORE: ";
        str += std::to_string(snake.get_score());

        ct += 1;
    }

    set_attr(CYAN, BLACK, false, false, false);
    printxy(str, Vector{(ssize_t) ncol, (ssize_t) nrow});
}

//---------------------------------------------------------

void View_text::draw_lose_msg()
{
    Vector wnsz = get_winsize_real();
    size_t cols = (size_t) wnsz.x();
    size_t rows = (size_t) wnsz.y();

    std::string lose_msg{"GAME OVER"};
    size_t ncol = (cols - lose_msg.size()) / 2;
    size_t nrow = rows / 2;

    set_attr(RED, BLACK, true, true, true);
    printxy(lose_msg, Vector{(ssize_t) ncol, (ssize_t) nrow});
}
