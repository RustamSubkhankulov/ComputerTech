#pragma once

//=========================================================

#include <functional>
#include <assert.h>

//---------------------------------------------------------

#include "../subs/subs.hpp"
#include "../model/model.hpp"
#include "../view/view.hpp"

//=========================================================

class Snake_controller
{
    private:

        Snake* snake_ = nullptr;
        Model* model_ = nullptr;

    public:
        
        Snake_controller() { return; }

        Snake_controller(Snake* snake, Model* model):
        snake_(snake),
        model_(model)
        {
            return;
        }

        virtual ~Snake_controller() 
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

class Snake_controller_AI : public Snake_controller, public Subscriber_on_timer
{
    public:

        Snake_controller_AI(unsigned upd_timeout):
        Subscriber_on_timer(upd_timeout)
        {
            subscribe_on_timer();
        }

        Snake_controller_AI(unsigned upd_timeout, Snake* snake, Model* model):
        Snake_controller(snake, model),
        Subscriber_on_timer(upd_timeout)
        {
            subscribe_on_timer();
        }

        void on_timer() override;

    private:

        void subscribe_on_timer();
};

//---------------------------------------------------------

class Snake_controller_smart_AI : public Snake_controller_AI
{
    private:

        enum Direction_type
        {
            NONE  = -1,
            FRONT =  0,
            LEFT  =  1,
            RIGHT =  2,
            DIRECTIONS_NUM
        };

        struct Direction 
        {
            Coords dir;
            bool safe;
        };

    public:

        Snake_controller_smart_AI():
        Snake_controller_AI(100U)
        { }

        Snake_controller_smart_AI(Snake* snake, Model* model):
        Snake_controller_AI(100U, snake, model) 
        { }

        void on_timer() override;
};

//---------------------------------------------------------

class Snake_controller_dumb_AI : public Snake_controller_AI
{
    public:

        Snake_controller_dumb_AI():
        Snake_controller_AI(100U)
        { }

        Snake_controller_dumb_AI(Snake* snake, Model* model):
        Snake_controller_AI(100U, snake, model) 
        { }

        void on_timer() override;
};

//---------------------------------------------------------

class Snake_controller_human: public Snake_controller, public Subscriber_on_key
{
    char lb_ = 0;
    char rb_ = 0;

    public:

        Snake_controller_human(char lb, char rb):
        lb_(lb),
        rb_(rb)
        {
            View* view = View::get_view();
            view->set_on_key(std::bind(&Snake_controller_human::on_key, this, std::placeholders::_1));
        }

        Snake_controller_human(char lb, char rb, Snake* snake, Model* model):
        Snake_controller(snake, model),
        lb_(lb),
        rb_(rb)
        {
            View* view = View::get_view();
            view->set_on_key(std::bind(&Snake_controller_human::on_key, this, std::placeholders::_1));
        }

        void on_key(int key) override;
};
