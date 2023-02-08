#pragma once 

//=========================================================

#include "../view/view.hpp"

//=========================================================

class View_text: public View
{
    public: 

        View_text()
            {}

        View_text            (const View_text& that) = default;
        View_text& operator= (const View_text& that) = default;
        ~View_text() override                        = default;

        void draw(void) const override;
};