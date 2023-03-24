#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

//---------------------------------------------------------

#include "../../include/view/view.hpp"
#include "../../include/controller/controller.hpp"
#include "../../include/model/model.hpp"

//---------------------------------------------------------

static void inv_arg_msg();

//=========================================================

int main(const int argc, const char** argv)
{
    if (argc != 2)
    {
        inv_arg_msg();
        exit(EXIT_FAILURE);
    }

    std::srand(std::time(0));

    View* view = View::get_view(argv[1]);
    assert(view != nullptr);
    
    Model model{};
    Vector field_size = view->get_winsize();

    Snake_controller_human human_ctrl {};

    Coords snake_start_pos{field_size.x() / 2, field_size.y() / 2};

    model.generate_snake(field_size, snake_start_pos, &human_ctrl);
    model.generate_rabbits(field_size, 10);

    view->set_model(&model);
    view->run_loop();

    delete view;

    exit(EXIT_SUCCESS);
}

static void inv_arg_msg()
{
    std::cerr << "Invalid parameters" << std::endl;
    std::cerr << "Usage: ./snake <mode>" << std::endl;

    return;
}