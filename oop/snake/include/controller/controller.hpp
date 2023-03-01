#pragma once

//=========================================================

#include <assert.h>

//---------------------------------------------------------

#include "../model/model.hpp"

//=========================================================

class Subscriber_on_key
{
    virtual void on_key(int key) = 0;
};

//---------------------------------------------------------

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

        virtual void on_key(int key) = 0;
};

//---------------------------------------------------------

class Snake_controller_AI: public Snake_controller
{
    public:

        Snake_controller_AI() { return; }

        Snake_controller_AI(Snake* snake, Model* model):
        Snake_controller(snake, model)
        {
            return;
        }

        void on_key(int key) override;
};

//---------------------------------------------------------

class Snake_controller_human: public Snake_controller
{
    public:

        Snake_controller_human() { return; }

        Snake_controller_human(Snake* snake, Model* model):
        Snake_controller(snake, model)
        {
            return;
        }

        void on_key(int key) override;
};
