#pragma once

//=========================================================

#include <utility>
#include <list>

//---------------------------------------------------------

#include "../vector/vector.hpp"

//=========================================================

using std::list;

using Coords     = Vector;
using Coords_list = std::list<Coords>;

//=========================================================

class Rabbit
{
    Coords coords_{};

    public:

        Rabbit(const Coords& coords)
        {
            coords_ = coords;
        }

        Rabbit& operator= (const Coords& coords)
        {
            coords_ = coords;
            return *this;
        }

        Rabbit(const Rabbit& rabbit)             = default;
        Rabbit& operator= (const Rabbit& rabbit) = default;
        ~Rabbit()                                = default;
        
        Coords get_coords() const
        {
            return coords_;
        }

        void set_coords(const Coords& coords) 
        {
            coords_ = coords;
        }

        void update(const Coords& field_size);
};

//---------------------------------------------------------

class Snake
{
    Coords_list coords_list_{};

    public:

        static constexpr int Snake_len = 7;

        Snake(const Coords_list& coords_list)
        {
            coords_list_ = coords_list;
        }

        Snake& operator= (const Coords_list& coords_list)
        {
            coords_list_ = coords_list;
            return *this;
        }

        Snake(const Snake& snake)             = default;
        Snake& operator= (const Snake& snake) = default;
        ~Snake()                              = default;

        Coords_list get_coords_list() const
        {
            return coords_list_;
        }

        void set_coords_list(const Coords_list& coords_list)
        {
            coords_list_ = coords_list;
        }

        void update(const Coords& field_size);
};

//---------------------------------------------------------

class Snake_controller;

class Model
{
    public:

        list<Rabbit> rabbits{};
        list<Snake> snakes{};

        Model()                              = default;
        Model            (const Model& that) = default;
        Model& operator= (const Model& that) = default;
        ~Model()                             = default;

        void update(const Coords& field_size);
        void generate_rabbits(const Coords& field_size, const int rnum);
        void generate_snake(const Coords& field_size, Snake_controller* controller);
};