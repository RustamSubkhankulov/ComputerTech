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

    std::srand((unsigned) std::time(0));

    View* view = View::get_view(argv[1]);
    assert(view != nullptr);
    
    Model model{};
    Vector field_size = view->get_winsize();

    Snake_controller_human human_ctrl1 {'a', 'd'};
    Snake_controller_human human_ctrl2 {'j', 'l'};

    // Snake_controller_dumb_AI  ai_ctrl1{};
    // Snake_controller_smart_AI ai_ctrl2{};
    // Snake_controller_dumb_AI ai_ctrl3{};

    Coords snake_start_pos1{field_size.x() / 2, field_size.y() / 2};
    Coords snake_start_pos2{snake_start_pos1.x(), snake_start_pos1.y() + 5};
    Coords snake_start_pos3{snake_start_pos1.x(), snake_start_pos1.y() + 10};

    // model.generate_snake(snake_start_pos1, &ai_ctrl1);
    // model.generate_snake(snake_start_pos2, &ai_ctrl2);
    // model.generate_snake(snake_start_pos3, &ai_ctrl3);

    model.generate_snake(snake_start_pos1, &human_ctrl1);
    model.generate_snake(snake_start_pos2, &human_ctrl2);

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