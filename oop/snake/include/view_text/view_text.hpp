#pragma once 

//=========================================================

#include <csignal>

//---------------------------------------------------------

#include "../view/view.hpp"

//=========================================================

class View_text: public View
{
    private:

        mutable Vector wnsz_{};
        struct sigaction oldact_{};

    public: 

        enum Color : int 
        {
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
            }

        View_text            (const View_text& that) = default;
        View_text& operator= (const View_text& that) = default;

        ~View_text() override
            {
                restore_sighandler();
                turn_on_carriage();
                cls();
                reset_attr();
            }

        Vector get_winsize() const override;

        void draw() override;

    private:

        void draw_frame();

        void putxy(const char sym, const Vector& coord);
        void printxy(const std::string& str, const Vector& coord);

        void hline(const size_t y, const size_t x0, const size_t x1, const char sym); 
        void vline(const size_t x, const size_t y0, const size_t y1, const char sym);

        void cls(); 

        void set_sighandler();
        void restore_sighandler();

        void reset_attr();
        void set_attr(enum Color fg, enum Color bg, bool bold, bool blink, bool underline);

        void turn_off_carriage();
        void turn_on_carriage();
};