#pragma once

//=========================================================

#include "../vector/vector.hpp"
#include "../model/model.hpp"

//---------------------------------------------------------

#include <string>
#include <list>
#include <functional>
#include <utility>

//=========================================================

class View
{
    protected: // ctor is private, all derivated class are not accessible for users (outside of view.cpp)

        View() { return; };
        std::list<std::function<void(int)>> subs_on_key {};
        std::list<std::pair<int, std::function<void(int)>>> subs_on_timer {};

    public:

        static View* current;
        static View* get_view(const std::string& what = std::string());

        virtual Vector get_winsize() const = 0;
        virtual void run_loop()  = 0;

        Model* set_model(Model* model)
        {
            Model* temp = model_;
            model_ = model;
            return temp; 
        }
        
        Model* get_model() { return model_; }

        void set_on_key(std::function<void(int)> callback)
        {
            subs_on_key.push_back(callback);
            return;
        }

        void set_on_timer(std::pair<int, std::function<void(int)>> callback)
        {
            subs_on_timer.push_back(callback);
            return; 
        }

        virtual ~View() { return; };

    private:

        Model* model_ = nullptr;

        View            (const View& that) = delete;
        View& operator= (const View& that) = delete;
};

//---------------------------------------------------------
