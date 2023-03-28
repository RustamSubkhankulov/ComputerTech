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
        sf::Texture snakesheet_texture;
        sf::Texture snake_skin_texture;
        sf::Texture snake_head_texture;

        static const unsigned Field_block_size = 32U;

    public: 

        View_graph();

        View_graph            (const View_graph& that) = delete;
        View_graph& operator= (const View_graph& that) = delete;

        ~View_graph() override;

        void field_sector_freed(const Coords& coords) override;
        Vector get_winsize() const override;
        void run_loop() override;

    private:

        void poll_events();
        void draw();
        void load_textures();

        int get_shortest_timeout();

        void draw_bg();
        void draw_frame();
        void draw_lose_msg();
        void draw_rabbits(Model* model);
        void draw_snakes(Model* model);
        void draw_results(Model* model);

        bool serve_subs_on_timer(sf::Int64 elapsed);
        void serve_subs_on_key(sf::Keyboard::Key key);
};