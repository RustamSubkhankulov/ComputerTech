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

using on_key_callback   = std::function<void(int)>;
using on_timer_callback = std::pair<unsigned, std::function<void(void)>>;

class View
{
    protected: // ctor is private, all derivated class are not accessible for users (outside of view.cpp)

        View() { return; };
        std::list<on_key_callback> subs_on_key {};
        std::list<on_timer_callback> subs_on_timer {};

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

        void set_on_key(on_key_callback callback)
        {
            subs_on_key.push_back(callback);
            return;
        }

        void set_on_timer(on_timer_callback callback)
        {
            subs_on_timer.push_back(callback);
            return; 
        }

        virtual void field_sector_freed(const Coords& coords) = 0;

        virtual ~View() { return; };

    private:

        Model* model_ = nullptr;

        View            (const View& that) = delete;
        View& operator= (const View& that) = delete;
};

//---------------------------------------------------------
