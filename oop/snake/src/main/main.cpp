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
    
    while(1)
        view->draw();

    return 0;
}