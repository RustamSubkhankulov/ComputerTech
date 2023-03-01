#include <iostream>

//---------------------------------------------------------

#include "../../include/view/view.hpp"
#include "../../include/view_graph/view_graph.hpp"
#include "../../include/view_text/view_text.hpp"

//=========================================================

View* View::current = nullptr;

//=========================================================

View* View::get_view(const std::string& what)
{
    if (current != nullptr)
    {
        if (!(what.empty()))
            return nullptr;

        return current;
    }

    if (what == "text")
        current = new View_text;
    else if (what == "graph")
        current = new View_graph;
    else 
        current = nullptr;

    return current;
}



