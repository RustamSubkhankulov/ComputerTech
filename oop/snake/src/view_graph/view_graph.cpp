#include <iostream>

//---------------------------------------------------------

#include "../../include/view_graph/view_graph.hpp"

//=========================================================

void View_graph::draw()  
{
    std::cout << "View_graph::draw called()" << std::endl;
}

//---------------------------------------------------------

Vector View_graph::get_winsize() const
{
    std::cout << "Not implemented yet" << std::endl;
    return Vector{};
}