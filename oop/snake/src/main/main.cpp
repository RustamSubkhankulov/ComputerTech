#include <iostream>
#include <string>

//---------------------------------------------------------

#include "../../include/view/view.hpp"
#include "../../include/controller/controller.hpp"
#include "../../include/model/model.hpp"

//=========================================================

int main(void)
{
    View* view = View::get_view("text");
    view->draw();

    return 0;
}