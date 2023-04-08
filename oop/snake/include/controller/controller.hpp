#pragma once

//=========================================================

#include <functional>
#include <assert.h>
#include <array>

//---------------------------------------------------------

#include "../subs/subs.hpp"
#include "../model/model.hpp"
#include "../view/view.hpp"

//=========================================================

class Snake_ctrl
{
    private:

        Snake* snake_ = nullptr;
        Model* model_ = nullptr;

    public:
        
        Snake_ctrl() { return; }

        Snake_ctrl(Snake* snake, Model* model):
        snake_(snake),
        model_(model)
        {
            return;
        }

        virtual ~Snake_ctrl() 
        { 
            return; 
        }

        Snake* set_snake(Snake* snake)
        {
            assert(snake);

            Snake* prev = snake_;
            snake_ = snake;

            return prev;
        }

        Model* set_model(Model* model)
        {
            assert(model);

            Model* prev = model_;
            model_ = model;

            return prev;
        }

        Snake* get_snake() { return snake_; }
        Model* get_model() { return model_; }
};

//---------------------------------------------------------

class Snake_ctrl_AI : public Snake_ctrl, public Subscriber_on_timer
{
    protected:

        enum Direction_type
        {
            NONE  = -1,
            FRONT =  0,
            LEFT  =  1,
            RIGHT =  2,
            DIRECTIONS_NUM
        };

        enum Movement_type
        {
            MIN  = 0,
            MAX  = 1
        };

        struct Direction 
        {
            Coords dir;
            Coords new_head;
            bool safe;
        };

        void get_directions(Snake* snake, Direction (&dirs) [DIRECTIONS_NUM]);
        void get_new_heads(const Coords& snake_head, Direction (&dirs) [DIRECTIONS_NUM]);
        void check_directions(Model* model, Direction (&dirs) [DIRECTIONS_NUM]);

        Direction_type get_random_safe_direction(Model* model, Snake* snake, const Coords& snake_head);

        Direction_type get_dir(Direction (&dirs) [DIRECTIONS_NUM], const Coords& snake_head, 
                                                                   const Coords& rabbit, Movement_type mov_type);
        Direction_type get_dir_keep(Direction (&dirs) [DIRECTIONS_NUM], const Coords& snake_head, const Coords& rabbit);

    public:

        Snake_ctrl_AI(unsigned upd_timeout):
        Subscriber_on_timer(upd_timeout)
        {
            subscribe_on_timer();
        }

        Snake_ctrl_AI(unsigned upd_timeout, Snake* snake, Model* model):
        Snake_ctrl(snake, model),
        Subscriber_on_timer(upd_timeout)
        {
            subscribe_on_timer();
        }

        void on_timer() override;

    private:

        void subscribe_on_timer();
};

//---------------------------------------------------------

class Snake_ctrl_smart_AI : public Snake_ctrl_AI
{
    public:

        enum Smart_AI_type
        {
            RIGHT_ANGLES = 0,
            DIAGONAL = 1,
            KEEP_DIR = 2
        };

    private:

        Smart_AI_type type_;

    public:

        Snake_ctrl_smart_AI(Smart_AI_type type = RIGHT_ANGLES):
        Snake_ctrl_AI(100U),
        type_(type)
        { }

        Snake_ctrl_smart_AI(Snake* snake, Model* model, Smart_AI_type type = RIGHT_ANGLES):
        Snake_ctrl_AI(100U, snake, model),
        type_(type)
        { }

        void on_timer() override;

    private:

        Coords get_closest_rabbit(Model* model, const Coords& snake_head);
};

//---------------------------------------------------------

class Snake_ctrl_dumb_AI : public Snake_ctrl_AI
{
    public:

        Snake_ctrl_dumb_AI():
        Snake_ctrl_AI(100U)
        { }

        Snake_ctrl_dumb_AI(Snake* snake, Model* model):
        Snake_ctrl_AI(100U, snake, model) 
        { }

        void on_timer() override;
};

//---------------------------------------------------------

class Snake_ctrl_human: public Snake_ctrl, public Subscriber_on_key
{
    char lb_ = 0;
    char rb_ = 0;

    public:

        Snake_ctrl_human(char lb, char rb):
        lb_(lb),
        rb_(rb)
        {
            View* view = View::get_view();
            view->set_on_key(std::bind(&Snake_ctrl_human::on_key, this, std::placeholders::_1));
        }

        Snake_ctrl_human(char lb, char rb, Snake* snake, Model* model):
        Snake_ctrl(snake, model),
        lb_(lb),
        rb_(rb)
        {
            View* view = View::get_view();
            view->set_on_key(std::bind(&Snake_ctrl_human::on_key, this, std::placeholders::_1));
        }

        void on_key(int key) override;
};
