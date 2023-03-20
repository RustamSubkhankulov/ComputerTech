#include <iostream>

//---------------------------------------------------------

#include "../../include/controller/controller.hpp"

//=========================================================

void Snake_controller_human::on_key(int key)
{
    switch(key)
    {
        case 'a':
        {
            Snake* snake = get_snake();
            snake->turn_left();
            break;
        }
        
        case 'd':
        {
            Snake* snake = get_snake();
            snake->turn_right();
            break;
        }

        default: break;
    }

    return;
}