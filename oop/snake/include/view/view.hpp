#pragma once

//=========================================================

#include "../vector/vector.hpp"
#include "../model/model.hpp"

//---------------------------------------------------------

#include <string>

//=========================================================

class View
{
    protected: // ctor is private, all derivated class are not accessible for users (outside of view.cpp)

        View() { return; };

    public:

        static View* current;
        static View* get_view(const std::string& what = std::string());

        virtual Vector get_winsize() const = 0;
        virtual void draw()  = 0;

        Model* set_model(Model* model)
        {
            Model* temp = model_;
            model_ = model;
            return temp; 
        }
        
        Model* get_model() { return model_; }

        virtual ~View() { return; };

    private:

        Model* model_ = nullptr;

        View            (const View& that) = delete;
        View& operator= (const View& that) = delete;
};

//---------------------------------------------------------
