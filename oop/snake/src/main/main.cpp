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
    
    Model model{};
    Vector field_size = view->get_winsize();

    model.generate_snakes(field_size, 1);  // 1 snake
    model.generate_rabbits(field_size, 3); // 3 rabbits

    view->set_model(&model);

    while(1)
        view->draw();

    return 0;
}