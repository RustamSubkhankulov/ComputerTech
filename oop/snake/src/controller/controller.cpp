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

    Direction dirs[DIRECTIONS_NUM] = {};

    switch(snake->direction_)
    {
        case Snake::Snake_dir::UP: 
        {
            dirs[FRONT].dir = Coords{ 0, -1};
            dirs[LEFT] .dir = Coords{+1,  0};
            dirs[RIGHT].dir = Coords{-1,  0};

            break; 
        }

        case Snake::Snake_dir::LEFT:
        {
            dirs[FRONT].dir = Coords{-1,  0};
            dirs[LEFT] .dir = Coords{ 0, +1};
            dirs[RIGHT].dir = Coords{ 0, -1};

            break;
        }

        case Snake::Snake_dir::RIGHT:
        {
            dirs[FRONT].dir = Coords{1,  0};
            dirs[LEFT] .dir = Coords{0, -1};
            dirs[RIGHT].dir = Coords{0, +1};

            break;
        }

        case Snake::Snake_dir::DOWN:
        {
            dirs[FRONT].dir = Coords{ 0, 1};
            dirs[LEFT] .dir = Coords{-1, 0};
            dirs[RIGHT].dir = Coords{+1, 0};

            break;
        }

        default: break;
    }

    dirs[FRONT].safe = model->field_is_free(snake_head + dirs[FRONT].dir);
    dirs[LEFT] .safe = model->field_is_free(snake_head + dirs[LEFT] .dir);
    dirs[RIGHT].safe = model->field_is_free(snake_head + dirs[RIGHT].dir);

    ssize_t delta_x = closest.x() - snake_head.x();
    ssize_t delta_y = closest.y() - snake_head.y();

    enum Direction_type high_prior = NONE;
    enum Direction_type mid_prior  = NONE;
    enum Direction_type low_prior  = NONE;

    if (abs(delta_x) <= abs(delta_y))
    {
        switch (snake->direction_)
        {
            case Snake::Snake_dir::UP: 
            {
                if (delta_x <= 0)
                {
                    high_prior = LEFT;
                    mid_prior  = FRONT;
                    low_prior  = RIGHT;
                }
                else 
                {
                    high_prior = RIGHT;
                    mid_prior  = FRONT;
                    low_prior  = LEFT;
                }

                break; 
            }

            case Snake::Snake_dir::LEFT:
            {
                if (delta_x <= 0)
                {
                    high_prior = FRONT;
                    mid_prior  = RIGHT;
                    low_prior  = LEFT;
                }
                else 
                {
                    high_prior = RIGHT;
                    mid_prior  = LEFT;
                    low_prior  = FRONT;
                }

                break;
            }

            case Snake::Snake_dir::RIGHT:
            {
                if (delta_x <= 0)
                {
                    high_prior = RIGHT;
                    mid_prior  = LEFT;
                    low_prior  = FRONT;
                }
                else 
                {
                    high_prior = FRONT;
                    mid_prior  = RIGHT;
                    low_prior  = LEFT;
                }

                break;
            }

            case Snake::Snake_dir::DOWN:
            {
                if (delta_x <= 0)
                {
                    high_prior = RIGHT;
                    mid_prior  = FRONT;
                    low_prior  = LEFT;
                }
                else 
                {
                    high_prior = LEFT;
                    mid_prior  = FRONT;
                    low_prior  = RIGHT;
                }

                break;
            }

            default: break;
        }
    }
    else // delta_x > delta_y
    {
        switch (snake->direction_)
        {
            case Snake::Snake_dir::UP: 
            {
                if (delta_y <= 0)
                {
                    high_prior = FRONT;
                    mid_prior  = RIGHT;
                    low_prior  = LEFT;
                }
                else 
                {
                    high_prior = RIGHT;
                    mid_prior  = LEFT;
                    low_prior  = FRONT;
                }

                break; 
            }

            case Snake::Snake_dir::LEFT:
            {
                if (delta_y <= 0)
                {
                    high_prior = RIGHT;
                    mid_prior  = FRONT;
                    low_prior  = LEFT;
                }
                else 
                {
                    high_prior = LEFT;
                    mid_prior  = FRONT;
                    low_prior  = RIGHT;
                }

                break;
            }

            case Snake::Snake_dir::RIGHT:
            {
                if (delta_y <= 0)
                {
                    high_prior = LEFT;
                    mid_prior  = FRONT;
                    low_prior  = RIGHT;
                }
                else 
                {
                    high_prior = RIGHT;
                    mid_prior  = FRONT;
                    low_prior  = LEFT;
                }

                break;
            }

            case Snake::Snake_dir::DOWN:
            {
                if (delta_y <= 0)
                {
                    high_prior = RIGHT;
                    mid_prior  = LEFT;
                    low_prior  = FRONT;
                }
                else 
                {
                    high_prior = FRONT;
                    mid_prior  = RIGHT;
                    low_prior  = LEFT;
                }

                break;
            }

            default: break;
        }
    }

    enum Direction_type chosen = NONE;

    if (dirs[high_prior].safe == true) 
        chosen = high_prior;
    else if (dirs[mid_prior].safe == true)
        chosen = mid_prior;
    else if (dirs[low_prior].safe == true)
        chosen = low_prior;

    if (chosen != NONE)
    {
        if (chosen == RIGHT)
            snake->turn_right();
        else if (chosen == LEFT)
            snake->turn_left();
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
