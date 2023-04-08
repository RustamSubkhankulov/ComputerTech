#include <iostream>
#include <stdexcept>

//---------------------------------------------------------

#include "../../include/controller/controller.hpp"

//---------------------------------------------------------

// #define DEBUG_CTRL

#ifdef DEBUG_CTRL 

    #define DEBUG_FPRINTF(...) fprintf(__VA_ARGS__)

#else 
    
    #define DEBUG_FPRINTF(...)

#endif 

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

    DEBUG_FPRINTF(stderr, "//=================================\n");
    DEBUG_FPRINTF(stderr, "closest x= %ld y = %ld \n", closest.x(), closest.y());

    Direction dirs[DIRECTIONS_NUM] = {};
    
    get_directions(snake, dirs);
    get_new_heads(snake_head, dirs);
    check_directions(model, dirs); 

    Direction_type dir = NONE;

    switch (type_)
    {
        case RIGHT_ANGLES: dir = get_dir(dirs, snake_head, closest, MIN); break;
        case DIAGONAL:     dir = get_dir(dirs, snake_head, closest, MAX); break;
        case KEEP_DIR:     dir = get_dir_keep(dirs, snake_head, closest); break;
        default: break;
    }

    if (dir == NONE)
        dir = get_random_safe_direction(model, snake, snake_head);

    DEBUG_FPRINTF(stderr, "direction: %d \n", (int) dir);

    switch (dir)
    {
        case LEFT : snake->turn_left(); break;
        case RIGHT: snake->turn_right(); break;
        case DIRECTIONS_NUM: break;
        case FRONT: break;
        case NONE:  break;
        default:    break;
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
    
    DEBUG_FPRINTF(stderr, "snake head now is x = %ld y = %ld \n", snake_head.x(), snake_head.y());

    for (unsigned iter = 0; iter < DIRECTIONS_NUM; iter++)
    {
        dirs[iter].new_head = snake_head + dirs[iter].dir;

        DEBUG_FPRINTF(stderr, "HEAD[%u] is x = %ld y = %ld \n", iter, dirs[iter].new_head.x(), dirs[iter].new_head.y());

    }
}

//---------------------------------------------------------

void Snake_ctrl_AI::get_directions(Snake* snake, Direction (&dirs) [DIRECTIONS_NUM])
{
    switch (snake->get_direction())
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

void Snake_ctrl_AI::check_directions(Model* model, Direction (&dirs) [DIRECTIONS_NUM])
{
    dirs[FRONT].safe = model->field_is_free_for_snake(dirs[FRONT].new_head);
    dirs[LEFT] .safe = model->field_is_free_for_snake(dirs[LEFT] .new_head);
    dirs[RIGHT].safe = model->field_is_free_for_snake(dirs[RIGHT].new_head);
}

//---------------------------------------------------------

Snake_ctrl_AI::Direction_type 
Snake_ctrl_AI::get_dir_keep(Direction (&dirs) [DIRECTIONS_NUM], const Coords& snake_head, const Coords& rabbit)
{
    Coords dist = rabbit - snake_head;

    DEBUG_FPRINTF(stderr, "dist: x = %ld y = %ld \n", dist.x(), dist.y());

    Direction_type direction = NONE;

    for (unsigned iter = 0; iter < DIRECTIONS_NUM; iter++)
    {
        if (dirs[iter].safe == false)
            continue;

        Coords new_dist = rabbit - dirs[iter].new_head;
        
        if (new_dist.len() < dist.len())
        {
            direction = (Direction_type) iter;
            break;
        }
    }

    DEBUG_FPRINTF(stderr, "get_dir() decided: %d \n", direction);
    return direction;
}

//---------------------------------------------------------

Snake_ctrl_AI::Direction_type
Snake_ctrl_AI::get_dir(Direction (&dirs) [DIRECTIONS_NUM], const Coords& snake_head, 
                                                           const Coords& rabbit, Snake_ctrl_AI::Movement_type mov_type)
{
    Coords dist = rabbit - snake_head;
    
    DEBUG_FPRINTF(stderr, "dist: x = %ld y = %ld \n", dist.x(), dist.y());

    Direction_type direction = NONE;

    for (unsigned iter = 0; iter < DIRECTIONS_NUM; iter++)
    {
        if (dirs[iter].safe == false)
            continue;

        Coords new_dist = rabbit - dirs[iter].new_head;

        DEBUG_FPRINTF(stderr, "new_dist[%u]: x = %ld y = %ld \n", iter, new_dist.x(), new_dist.y());

        if ((abs(dist.y()) == 0 && abs(dist.x()) != 0)
         || (mov_type == MIN && abs(dist.x()) != 0 && abs(dist.x()) <= abs(dist.y()))
         || (mov_type == MAX && abs(dist.x()) >= abs(dist.y()))) // decreasing delta x
        {

            DEBUG_FPRINTF(stderr, "decreasing delta x \n");

            if (abs(new_dist.x()) < abs(dist.x()))
            {
                direction = (Direction_type) iter;
                break;
            }
        }
        else if ((abs(dist.x()) == 0 && abs(dist.y()) != 0)
              || (mov_type == MAX && abs(dist.x()) <= abs(dist.y()))
              || (mov_type == MIN && abs(dist.x()) >= abs(dist.y())))  
        {

            DEBUG_FPRINTF(stderr, "decreasing delta y \n");

            if (abs(new_dist.y()) < abs(dist.y()))
            {
                direction = (Direction_type) iter;
                break;
            }
        }
    }

    DEBUG_FPRINTF(stderr, "get_dir() decided: %d \n", direction);
    return direction;
}

//---------------------------------------------------------

Snake_ctrl_AI::Direction_type 
Snake_ctrl_AI::get_random_safe_direction(Model* model, Snake* snake, const Coords& snake_head)
{
    Direction dirs[DIRECTIONS_NUM] = {};
    
    get_directions(snake, dirs);
    get_new_heads(snake_head, dirs);
    check_directions(model, dirs);

    if (dirs[FRONT].safe == false 
     && dirs[RIGHT].safe == false
     && dirs[LEFT ].safe == false)
        return NONE;

    Direction_type random = NONE;

    do 
    {
        random = (Direction_type) ((unsigned) std::rand() % DIRECTIONS_NUM);

    } while (dirs[random].safe != true);

    return random;
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
