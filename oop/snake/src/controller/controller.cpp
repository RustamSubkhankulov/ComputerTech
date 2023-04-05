#include <iostream>
#include <stdexcept>

//---------------------------------------------------------

#include "../../include/controller/controller.hpp"

//=========================================================

void Snake_ctrl_human::on_key(int key)
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

void Snake_ctrl_AI::subscribe_on_timer()
{
    View* view = View::get_view();

    on_timer_callback callback{};

    callback.first  = Subscriber_on_timer::timeout;
    callback.second = std::bind(&Snake_ctrl_AI::on_timer, this);

    view->set_on_timer(callback);
}

//---------------------------------------------------------

void Snake_ctrl_AI::on_timer() 
{
    subscribe_on_timer();
}

//---------------------------------------------------------

void Snake_ctrl_smart_AI::on_timer() 
{
    Model* model = get_model();
    Snake* snake = get_snake();

    if (model == nullptr)
        throw std::runtime_error("controller has no model");

    if (snake == nullptr)
        throw std::runtime_error("controller has no snake");

    if (snake->is_alive() == false)
    {
        Snake_ctrl_AI::on_timer();
        return;
    }

    Coords_list snake_coord_list = snake->get_coords_list();
    Coords snake_head = snake_coord_list.back();

    if (model->rabbits.size() == 0)
    {
        Direction_type random = get_random_safe_direction(model, snake, snake_head);

        if (random == RIGHT)
            snake->turn_right();
        else if (random == LEFT)
            snake->turn_left();

        Snake_ctrl_AI::on_timer();
        return;
    }

    Coords closest = get_closest_rabbit(model, snake_head);

    Direction dirs[DIRECTIONS_NUM] = {};
    
    get_directions(snake, dirs);
    check_directions(model, dirs, snake_head); // TODO 
    get_new_heads(snake_head, dirs);

    Direction_type dir = NONE;

    if (type_ == RIGHT_ANGLES)
        dir = get_dir_min(dirs, snake_head, closest);
    else if (type_ == DIAGONAL)
        dir = get_dir_max(dirs, snake_head, closest);

    // TODO

    switch(dir)
    {
        case LEFT : snake->turn_left(); break;
        case RIGHT: snake->turn_left(); break;
        case FRONT: rbeak;
        default: break;
    }

    Snake_ctrl_AI::on_timer();
}

//---------------------------------------------------------

Coords Snake_ctrl_smart_AI::get_closest_rabbit(Model* model, const Coords& snake_head)
{
    Coords closest = model->rabbits.front().get_coords();

    for (const auto& rabbit : model->rabbits)
    {
        Coords rabbit_coords = rabbit.get_coords();

        if ((rabbit_coords - snake_head).len() < (closest - snake_head).len())
            closest = rabbit_coords;
    }

    return closest;
}

//---------------------------------------------------------

void Snake_ctrl_AI::get_new_heads(const Coords& snake_head, Direction (&dirs) [DIRECTIONS_NUM])
{
    for (unsigned iter = 0; iter < DIRECTIONS_NUM; iter++)
    {
        dirs[iter].new_head = snake_head + dirs[iter].dir;
    }
}

//---------------------------------------------------------

void Snake_ctrl_AI::get_directions(Snake* snake, Direction (&dirs) [DIRECTIONS_NUM])
{
    switch(snake->get_direction())
    {
        case Snake::Snake_dir::UP: 
        {
            dirs[FRONT].dir = Coords{ 0, -1};
            dirs[LEFT] .dir = Coords{-1,  0};
            dirs[RIGHT].dir = Coords{+1,  0};

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
            dirs[LEFT] .dir = Coords{+1, 0};
            dirs[RIGHT].dir = Coords{-1, 0};

            break;
        }

        default: break;
    }
}

//---------------------------------------------------------

void Snake_ctrl_AI::check_directions(Model* model, Direction (&dirs) [DIRECTIONS_NUM], const Coords& snake_head)
{
    dirs[FRONT].safe = model->field_is_free_for_snake(snake_head + dirs[FRONT].dir);
    dirs[LEFT] .safe = model->field_is_free_for_snake(snake_head + dirs[LEFT] .dir);
    dirs[RIGHT].safe = model->field_is_free_for_snake(snake_head + dirs[RIGHT].dir);
}

//---------------------------------------------------------

Snake_ctrl_AI::Direction_type
Snake_ctrl_AI::get_dir_min(Direction (&dirs) [DIRECTIONS_NUM], const Coords& snake_head, 
                                                               const Coords& rabbit)
{
    Coords dist = rabbit - snake_head;

    
}

//---------------------------------------------------------

Snake_ctrl_AI::Direction_type
Snake_ctrl_AI::get_dir_max(Direction (&dirs) [DIRECTIONS_NUM], const Coords& snake_head, 
                                                               const Coords& rabbit)
{
    Coords dist = rabbit - snake_head;



}

//---------------------------------------------------------

Snake_ctrl_AI::Direction_type
Snake_ctrl_AI::get_dir_keep(Direction (&dirs) [DIRECTIONS_NUM], const Coords& snake_head, 
                                                                const Coords& rabbit)
{

}

//---------------------------------------------------------

Snake_ctrl_AI::Direction_type 
Snake_ctrl_AI::get_random_safe_direction(Model* model, Snake* snake, const Coords& snake_head)
{
    Direction dirs[DIRECTIONS_NUM] = {};
    
    get_directions(snake, dirs);
    check_directions(model, dirs, snake_head);

    if (dirs[FRONT].safe == false 
     && dirs[RIGHT].safe == false
     && dirs[LEFT ].safe == false)
        return NONE;

    Direction_type random = NONE;

    do 
    {
        random = (Direction_type) ((unsigned) std::rand() % DIRECTIONS_NUM);

    } while (dirs[random].safe != true);

    return (Direction_type) random;
}

//---------------------------------------------------------

void Snake_ctrl_dumb_AI::on_timer() 
{
    Model* model = get_model();
    Snake* snake = get_snake();

    if (model == nullptr)
        throw std::runtime_error("controller has no model");

    if (snake == nullptr)
        throw std::runtime_error("controller has no snake");

    Coords_list snake_coord_list = snake->get_coords_list();
    Coords snake_head = snake_coord_list.back();

    Direction_type random = get_random_safe_direction(model, snake, snake_head);

    if (random == RIGHT)
        snake->turn_right();
    else if (random == LEFT)
        snake->turn_left();

    Snake_ctrl_AI::on_timer();
}
