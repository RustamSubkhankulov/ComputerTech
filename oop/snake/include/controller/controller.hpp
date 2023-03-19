#pragma once

//=========================================================

#include <functional>
#include <assert.h>

//---------------------------------------------------------

#include "../subs/subs.hpp"
#include "../model/model.hpp"
#include "../view/view.hpp"

//=========================================================

class Snake_controller: public Subscriber_on_key
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

        virtual void on_key(int key) { return; };
};

//---------------------------------------------------------

class Snake_controller_AI: public Snake_controller
{
    public:

        Snake_controller_AI() 
        {
            View* view = View::get_view();
            view->set_on_key(std::bind(&Snake_controller_AI::on_key, this, std::placeholders::_1));
        }

        Snake_controller_AI(Snake* snake, Model* model):
        Snake_controller(snake, model)
        {
            View* view = View::get_view();
            view->set_on_key(std::bind(&Snake_controller_AI::on_key, this, std::placeholders::_1));
        }

        void on_key(int key) override;
};

//---------------------------------------------------------

class Snake_controller_human: public Snake_controller
{
    public:

        Snake_controller_human()
        {
            View* view = View::get_view();
            view->set_on_key(std::bind(&Snake_controller_human::on_key, this, std::placeholders::_1));
        }

        Snake_controller_human(Snake* snake, Model* model):
        Snake_controller(snake, model)
        {
            View* view = View::get_view();
            view->set_on_key(std::bind(&Snake_controller_human::on_key, this, std::placeholders::_1));
        }

        void on_key(int key) override;
};
