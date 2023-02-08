#pragma once

//=========================================================

#include <string>

//=========================================================

class View
{
    public:

        static View* current;

        static View* get_view(const std::string& what = std::string());

        virtual void draw(void) const = 0;
        virtual ~View() { return; };
};

//---------------------------------------------------------
