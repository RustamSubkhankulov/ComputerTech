#pragma once 

//=========================================================

#include "../view/view.hpp"

//=========================================================

class View_text: public View
{
    // Vector winsize{}; // TODO resizes, colors, turn off cursor & on of dtor

    public: 

        View_text()
            {}

        View_text            (const View_text& that) = default;
        View_text& operator= (const View_text& that) = default;
        ~View_text() override                        = default;

        Vector get_winsize() const override;

        void draw() override;

    private:

        void draw_frame();

        void putxy(const char sym, const Vector& coord);

        void hline(const size_t y, const size_t x0, const size_t x1, const char sym); 
        void vline(const size_t x, const size_t y0, const size_t y1, const char sym);

        void cls(); 
};