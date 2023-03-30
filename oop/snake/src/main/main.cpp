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

    // Snake_ctrl_human human_ctrl1 {'a', 'd'};
    // Snake_ctrl_human human_ctrl2 {'j', 'l'};

    Snake_ctrl_dumb_AI dumb_ai{};
    Snake_ctrl_smart_AI smart_ai1{Snake_ctrl_smart_AI::RIGHT_ANGLES};
    Snake_ctrl_smart_AI smart_ai2{Snake_ctrl_smart_AI::DIAGONAL};

    Coords snake_start_pos1{field_size.x() / 2, field_size.y() / 2};
    Coords snake_start_pos2{snake_start_pos1.x(), snake_start_pos1.y() + 5};
    Coords snake_start_pos3{snake_start_pos1.x(), snake_start_pos1.y() + 10};

    model.generate_snake(snake_start_pos1, &dumb_ai);
    model.generate_snake(snake_start_pos2, &smart_ai1);
    model.generate_snake(snake_start_pos3, &smart_ai2);

    // model.generate_snake(snake_start_pos1, &human_ctrl1);
    // model.generate_snake(snake_start_pos2, &human_ctrl2);

    model.generate_rabbits(field_size, 100);

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