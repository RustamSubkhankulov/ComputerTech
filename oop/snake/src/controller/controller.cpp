#include <iostream>
#include <stdexcept>

//---------------------------------------------------------

#include "../../include/controller/controller.hpp"

//=========================================================

void Snake_controller_human::on_key(int key)
{
    if (key == lb_)
    {
        Snake* snake = get_snake();

        if (snake == nullptr)
            throw std::runtime_error{"controller has no snake"};

        snake->turn_left();
    }
    else if (key == rb_)
    {
        Snake* snake = get_snake();

        if (snake == nullptr)
            throw std::runtime_error{"controller has no snake"};

        snake->turn_right();
    }

    return;
}

//---------------------------------------------------------

void Snake_controller_AI::subscribe_on_timer()
{
    View* view = View::get_view();

    on_timer_callback callback{};

    callback.first  = Subscriber_on_timer::timeout;
    callback.second = std::bind(&Snake_controller_AI::on_timer, this);

    view->set_on_timer(callback);
}

//---------------------------------------------------------

void Snake_controller_AI::on_timer() 
{
    subscribe_on_timer();
}

//---------------------------------------------------------

void Snake_controller_smart_AI::on_timer() 
{
    Model* model = get_model();
    Snake* snake = get_snake();

    if (model == nullptr)
        throw std::runtime_error("controller has no model");

    if (snake == nullptr)
        throw std::runtime_error("controller has no snake");

    Coords_list snake_coord_list = snake->get_coords_list();
    Coords snake_head = snake_coord_list.back();

    Coords closest = model->rabbits.front().get_coords();

    for (const auto& rabbit : model->rabbits)
    {
        Coords rabbit_coords = rabbit.get_coords();

        if ((rabbit_coords - snake_head).len() < (closest - snake_head).len())
            closest = rabbit_coords;
    }

    switch(snake->direction_)
    {
        case Snake::Snake_dir::UP:
        {
            if (closest.y() < snake_head.y())
            {
                snake->turn_right();
            }
            else 
            {
                if (closest.x() < snake_head.x())
                {
                    snake->turn_left();
                }
                else if (closest.x() > snake_head.x())
                {
                    snake->turn_right();
                }
            }

            break;
        }
        case Snake::Snake_dir::LEFT:
        {
            if (closest.x() > snake_head.x())
            {
                snake->turn_right();
            }
            else 
            {
                if (closest.y() < snake_head.y())
                {
                    snake->turn_left();
                }
                else if (closest.y() > snake_head.y())
                {
                    snake->turn_right();
                }
            }

            break;
        }
        case Snake::Snake_dir::RIGHT:
        {
            if (closest.x() < snake_head.x())
            {
                snake->turn_right();
            }
            else 
            {
                if (closest.y() < snake_head.y())
                {
                    snake->turn_right();
                }
                else if (closest.y() > snake_head.y())
                {
                    snake->turn_left();
                }
            }

            break;
        }
        case Snake::Snake_dir::DOWN:
        {
            if (closest.y() > snake_head.y())
            {
                snake->turn_right();
            }
            else 
            {
                if (closest.x() < snake_head.x())
                {
                    snake->turn_right();
                }
                else if (closest.x() > snake_head.x())
                {
                    snake->turn_left();
                }
            }

            break;
        }
        default: break;
    }

    Snake_controller_AI::on_timer();
}

//---------------------------------------------------------

void Snake_controller_dumb_AI::on_timer() 
{
    Model* model = get_model();
    Snake* snake = get_snake();

    if (model == nullptr)
        throw std::runtime_error("controller has no model");

    if (snake == nullptr)
        throw std::runtime_error("controller has no snake");

    Coords_list snake_coord_list = snake->get_coords_list();
    Coords snake_head = snake_coord_list.back();

    Coords dir = snake->snake_dir_to_coords();
    Coords new_head = snake_head + dir;

    View* view = View::get_view();
    Vector wnsz = view->get_winsize();

    int random = std::rand() % 5;

    if (random == 1)
        snake->turn_left();
    else if (random == 2)
        snake->turn_right();

    Snake_controller_AI::on_timer();
}
