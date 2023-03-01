#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <iostream>

//---------------------------------------------------------

#include "../../include/model/model.hpp"
#include "../../include/view/view.hpp"
#include "../../include/controller/controller.hpp"

//=========================================================

using std::rand;

//=========================================================

// TODO start snake pos

static int get_rand_offs()
{
    return (rand() % 3) - 1;
}

//---------------------------------------------------------

static Coords get_rand_dir()
{
    return Coords{get_rand_offs(), get_rand_offs()};
}

//---------------------------------------------------------

static Coords get_rand_coords(const Vector& field_size)
{
    return Coords{rand() % field_size.x(), rand() & field_size.y()};
} 

//---------------------------------------------------------

static Coords_list get_rand_coords_list(const Vector& field_size, size_t len)
{
    Coords_list coords_list{};

    Coords start = get_rand_coords(field_size);
    coords_list.push_back(start);

    Coords prev = start;

    for (unsigned iter = 0; iter < len - 1; iter++)
    {
        Coords dir{};
        Coords cur{};

    again:
        dir = get_rand_dir();

        if (!(dir.x() == 0 || dir.y() == 0))
            goto again;

        if ((prev.x() == 0 && dir.x() == -1)
          || prev.x() == field_size.x() - 1 && dir.x() == 1) // TODO class Coords
            dir.set_x(0);

        if ((prev.y() == 0 && dir.y() == -1)
          || prev.y() == field_size.y() - 1 && dir.y() == 1)
            dir.set_y(0);

        if (dir.x() == 0 && dir.y() == 0)
            goto again;

        cur = prev + dir;

        for (const Coords& elem : coords_list)
        {
            if (cur.x() == elem.x() && cur.y() == prev.y())
                goto again;
        }

        coords_list.push_back(cur);
        prev = cur;
    }

    return coords_list;
}

//---------------------------------------------------------

void Rabbit::update(const Vector& field_size)
{
    Coords dir{};

    while (1)
    {
        dir = get_rand_dir();

        if ((coords_.x() == 0 && dir.x() == -1)
          || coords_.x() == field_size.x() - 1 && dir.x() == 1) // TODO class Coords
            dir.set_x(0);

        if ((coords_.y() == 0 && dir.y() == -1)
          || coords_.y() == field_size.y() - 1 && dir.y() == 1)
            dir.set_y(0);

        if (!(dir.x() == 0 && dir.y() == 0))
            break;
    }

    coords_ += dir;
    return;
}

//---------------------------------------------------------

void Snake::update(const Vector& field_size)
{
    Coords head = coords_list_.back();
    
    Coords dir{};
    Coords new_head{};

    if (direction_ == Snake_dir::DOWN)
    {
        dir = {0, 1};
    }
    else if (direction_ == Snake_dir::RIGHT)
    {   
        dir = {1, 0};
    }
    else if (direction_ == Snake_dir::UP)
    {
        dir = {0, -1};
    }
    else if (direction_ == Snake_dir::LEFT)
    {
        dir = {-1, 0};
    }

    if ((head.x() == 0 && dir.x() == -1)
        || head.x() == field_size.x() - 1 && dir.x() == 1) // TODO class Coords
        dir.set_x(0);

    if ((head.y() == 0 && dir.y() == -1)
        || head.y() == field_size.y() - 1 && dir.y() == 1)
        dir.set_y(0);

    if (dir.x() == 0 && dir.y() == 0)
        assert(false);

    new_head = head + dir;
    
    for (const Coords& coords : coords_list_)
    {
        if (coords.x() == new_head.x() && coords.y() == new_head.y())
            assert(false);
    }

    coords_list_.pop_front();
    coords_list_.push_back(new_head);
}

//---------------------------------------------------------

void Model::update(const Vector& field_size)
{
    for (Rabbit& rabbit : rabbits)
        rabbit.update(field_size);
    
    for (Snake& snake : snakes)
        snake.update(field_size);
}

//---------------------------------------------------------

void Model::generate_rabbits(const Vector& field_size, const int rnum)
{
    for (unsigned iter = 0; iter < rnum; iter++)
    {
        Rabbit cur = Vector{0, 0};
        Coords cur_coords{};

    again:

        cur_coords = get_rand_coords(field_size);

        for (const Rabbit& rabbit : rabbits)
        {
            Coords coords = rabbit.get_coords();

            if (cur_coords.x() == coords.x() && cur_coords.y() == coords.y())
                goto again;
        }

        cur = cur_coords;
        rabbits.push_back(cur);
    }
}

//---------------------------------------------------------

void Snake::turn_left()
{
    if (direction_ == Snake_dir::DOWN)
    {
        direction_ = Snake_dir::RIGHT;
    }
    else if (direction_ == Snake_dir::RIGHT)
    {   
        direction_ = Snake_dir::UP;
    }
    else if (direction_ == Snake_dir::UP)
    {
        direction_ = Snake_dir::LEFT;
    }
    else if (direction_ == Snake_dir::LEFT)
    {
        direction_ = Snake_dir::DOWN;
    }

    return;
}

//---------------------------------------------------------

void Snake::turn_right()
{
    if (direction_ == Snake_dir::DOWN)
    {
        direction_ = Snake_dir::LEFT;
    }
    else if (direction_ == Snake_dir::RIGHT)
    {   
        direction_ = Snake_dir::DOWN;
    }
    else if (direction_ == Snake_dir::UP)
    {
        direction_ = Snake_dir::RIGHT;
    }
    else if (direction_ == Snake_dir::LEFT)
    {
        direction_ = Snake_dir::UP;
    }

    return;
}

//---------------------------------------------------------

void Model::generate_snake(const Vector& field_size, Snake_controller* controller)
{
    assert(controller);

    Snake snake = get_rand_coords_list(field_size, Snake::Snake_len);

    controller->set_model(this);
    snakes.push_back(snake);
    controller->set_snake(&snakes.back());

    return;
}
