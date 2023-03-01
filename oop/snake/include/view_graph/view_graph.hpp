#pragma once 

//=========================================================

#include "../view/view.hpp"

//=========================================================

class View_graph: public View 
{
    public: 

        View_graph()
            {}

        View_graph            (const View_graph& that) = default;
        View_graph& operator= (const View_graph& that) = default;
        ~View_graph() override                         = default;

        Vector get_winsize() const override;

        void run_loop() override;
};