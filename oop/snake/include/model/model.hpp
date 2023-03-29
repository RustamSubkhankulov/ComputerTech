#pragma once

//=========================================================

#include <utility>
#include <list>

//---------------------------------------------------------

#include "../vector/vector.hpp"
#include "../subs/subs.hpp"

//=========================================================

using std::list;

using Coords     = Vector;
using Coords_list = std::list<Coords>;

//=========================================================

class Snake_controller;
class Rabbit;
class Snake;

class Model : public Subscriber_on_timer
{
    private:

        bool model_updated = false;
        bool game_ended = false;

    public:

        list<Rabbit> rabbits{};
        list<Snake> snakes{};

        Model():
        Subscriber_on_timer(100U)
        {
            subscribe_on_timer();
        }

        Model            (const Model& that) = default;
        Model& operator= (const Model& that) = default;
        ~Model()                             = default;

        void generate_rabbits(const Coords& field_size, const int rnum);
        void generate_snake(Coords& start_pos, Snake_controller* controller);
        bool field_is_free(const Coords& coords);

        bool model_is_updated() const
        {
            return model_updated;
        }

        void model_aknowledge_update()
        {
            model_updated = false;
        }

        bool game_is_ended() const
        {
            return game_ended;
        }

    private:

        void subscribe_on_timer();
        void on_timer() override;
        void update(const Coords& field_size);
        void process_events();
};

//---------------------------------------------------------

class Rabbit
{
    friend class Model;

    unsigned update_ct = 0;
    enum : unsigned { UPDATE_FREQ = 10U };
    bool alive = true;

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

        bool is_alive() const
        {
            return alive;
        }

        void update(const Coords& field_size, const list<Rabbit>& rabbits,
                                              const list<Snake>& snakes);
};

//---------------------------------------------------------

class Snake_controller_smart_AI;
class Snake_controller_dumb_AI;

class Snake
{
    friend class Model;
    friend class Snake_controller_smart_AI;
    friend class Snake_controller_dumb_AI;

    enum class Snake_dir
    {
        UP = 1, LEFT = 2, RIGHT = 3, DOWN = 4
    };

    Snake_dir direction_ = Snake_dir::RIGHT;
    Coords_list coords_list_{};
    unsigned score = 0;
    bool alive = true;

    unsigned make_longer_ct = 0;

    public:

        enum Update_res
        {
            OK = 0,
            LOSE = 1
        };

        static constexpr unsigned Snake_start_len = 7;

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

        unsigned get_score() const
        {
            return score;
        }

        bool is_alive() const
        {
            return alive;
        }

        void make_longer()
        {
            make_longer_ct += 1;
        }

        Snake::Update_res update(const Coords& field_size);

        void turn_left();
        void turn_right();

    private:

        Coords snake_dir_to_coords();

};

//---------------------------------------------------------
