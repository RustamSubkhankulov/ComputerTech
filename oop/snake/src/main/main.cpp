#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

//---------------------------------------------------------

#include "../../include/view/view.hpp"
#include "../../include/controller/controller.hpp"
#include "../../include/model/model.hpp"

//=========================================================

int main(void)
{
    std::srand(std::time(0));

    View* view = View::get_view("text");
    
    Model model{};
    Vector field_size = view->get_winsize();

    Snake_controller_human human_ctrl {};

    model.generate_snake(field_size, &human_ctrl);
    model.generate_rabbits(field_size, 10);

    view->set_model(&model);
    view->run_loop();

    return 0;
}