#include <iostream>

//---------------------------------------------------------

#include "../../include/view/view.hpp"

//=========================================================

static View* View::get_view(const std::string& what = std::string())
{
    if (current != nullptr)
        return current;

    if (what == "text")
        return new View_text;
    else if (what == "graph")
        return new View_graph
    else 
        return nullptr;
}



