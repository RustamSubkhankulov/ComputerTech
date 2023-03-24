#pragma once 

//=========================================================

#include <string>

#include "../view/view.hpp"
#include <SFML/Graphics.hpp>

//=========================================================

class View_graph: public View 
{
    private:

        sf::RenderWindow wndw;

        sf::Texture bg_texture;
        sf::Texture rabbit_texture;
        sf::Texture rabbitsheet_texture;

    public: 

        View_graph();

        View_graph            (const View_graph& that) = delete;
        View_graph& operator= (const View_graph& that) = delete;

        ~View_graph() override;

        Vector get_winsize() const override;
        void run_loop() override;

    private:

        void load_textures();
};