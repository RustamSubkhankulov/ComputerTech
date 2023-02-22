#include <cstdlib>
#include <ctime>

//---------------------------------------------------------

#include "../../include/model/model.hpp"

//=========================================================

using std::srand;
using std::rand;
using std::time;

//=========================================================

// TODO overlapping objects

static int get_rand_offs()
{
    srand(time(0));
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
    srand(time(0));
    return Coords{rand() % field_size.x(), rand() & field_size.y()};
} 

//---------------------------------------------------------

static Coords_list get_rand_coords_list(const Vector& field_size, size_t len)
{
    Coords_list coords_list{};

    Coords start = get_rand_coords(field_size);
    coords_list.push_back(start);

    Coords* prev = &start;

    for (unsigned iter = 0; iter < len - 1; iter++)
    {
        Coords dir{};
        Coords cur{};

    again:
        dir = get_rand_dir();

        if (dir.x() == 1 && dir.y() == 1)
            goto again;

        if ((prev->x() == 0 && dir.x() == -1)
          || prev->x() == field_size.x() - 1 && dir.x() == 1) // TODO class Coords
            dir.set_x(0);

        if ((prev->y() == 0 && dir.y() == -1)
          || prev->y() == field_size.y() - 1 && dir.y() == 1)
            dir.set_y(0);

        if (dir.x() == 0 && dir.y() == 0)
            goto again;

        cur = *prev + dir;

        if (cur.x() == prev->x() && cur.y() == prev->y())
            goto again;

        coords_list.push_back(cur);
        prev = &cur;
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

    while (1)
    {
        dir = get_rand_dir();

        if (dir.x() == 1 && dir.y() == 1)
            continue;

        if ((head.x() == 0 && dir.x() == -1)
          || head.x() == field_size.x() - 1 && dir.x() == 1) // TODO class Coords
            dir.set_x(0);

        if ((head.y() == 0 && dir.y() == -1)
          || head.y() == field_size.y() - 1 && dir.y() == 1)
            dir.set_y(0);

        if (dir.x() == 0 && dir.y() == 0)
            continue;

        new_head = head + dir;
        if (!(new_head.x() == head.x() && new_head.y() == head.y()))
            break;
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

void Model::generate_snakes(const Vector& field_size, const int snum) // TODO overlapping snakes
{
    for (unsigned iter = 0; iter < snum; iter++)
    {
        Snake cur = get_rand_coords_list(field_size, Snake::Snake_len);
        snakes.push_back(cur);
    }
}
