#pragma once

//=========================================================

#include "../vector/vector.hpp"

//---------------------------------------------------------

#include <string>

//=========================================================

class View
{
    protected: // ctor is private, all derivated class are not accessible for users (outside of view.cpp)

        View() { return; };

    public:

        static View* current;

        static View* get_view(const std::string& what = std::string());

        virtual Vector get_winsize() const = 0;
        virtual void draw()  = 0;
        
        virtual ~View() { return; };

    private:

        View            (const View& that) = default;
        View& operator= (const View& that) = default;
};

//---------------------------------------------------------
