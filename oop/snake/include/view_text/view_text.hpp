#pragma once 

//=========================================================

#include <csignal>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

//---------------------------------------------------------

#include "../view/view.hpp"

//=========================================================

class View_text: public View
{
    private:

        mutable Vector wnsz_{};
        struct sigaction oldact_{ 0 };
        struct termios termis_attr{ 0 };

        bool exit = false;

    public: 

        enum Color : int 
        {
            NON_PRESENT = -1,
            BLACK   = 0,
            RED     = 1,
            GREEN   = 2,
            BROWN   = 3,
            BLUE    = 4,
            MAGENTA = 5,
            CYAN    = 6,
            WHITE   = 7,
            DEF     = 9
        };

        View_text():
            wnsz_(get_winsize())
            {
                set_sighandler();
                turn_off_carriage();
                termios_change_conf();
            }

        View_text            (const View_text& that) = delete;
        View_text& operator= (const View_text& that) = delete;

        ~View_text() override
            {
                restore_sighandler();
                turn_on_carriage();
                reset_attr();
                cls();
                set_location(Vector{1,1});
                termios_restore_conf();
            }
            
        void field_sector_freed(const Coords& coords) override;
        Vector get_winsize() const override;

        void run_loop() override;

    private:

        struct Field
        {
            char sym;
            enum Color color;

            bool operator== (const Field& that) { return ((sym == that.sym) && (color == that.color)); };
            bool operator!= (const Field& that) { return ((sym != that.sym) || (color != that.color)); };
        };

        void poll_events(int timeout);
        void poll_on_key(int timeout);
        int  get_poll_timeout();
        void serve_subs_on_timer(const timeval& elapsed);

        timeval get_cur_time() const;

        Vector get_winsize_real() const;
        void draw_frame();
        void draw_lose_msg();
        
        void set_location(const Vector& coord);
        void putxy(const char sym, const Vector& coord);
        void printxy(const std::string& str, const Vector& coord);

        void putxy_to_shot(const char sym, Color color, const Vector& coord);
        void realloc_shots();
        void show_shot();

        void hline(const size_t y, const size_t x0, const size_t x1, const char sym); 
        void vline(const size_t x, const size_t y0, const size_t y1, const char sym);

        void cls(); 

        void set_sighandler();
        void restore_sighandler() const;

        void reset_attr();
        void set_attr(enum Color fg, enum Color bg, bool bold, bool blink, bool underline);

        void turn_off_carriage() const;
        void turn_on_carriage() const;

        void termios_change_conf();
        void termios_restore_conf();

        void draw_bg();
        void draw_results(Model* model);
        void draw_rabbits(Model* model);
        void draw_snakes(Model* model);
};