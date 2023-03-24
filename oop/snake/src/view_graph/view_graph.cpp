#include <iostream>

//---------------------------------------------------------

#include "../../include/view_graph/view_graph.hpp"

//=========================================================

static const sf::Vector2u Wndw_start_size{600, 600};

static const sf::Uint32 Wndw_start_style = sf::Style::Resize
                                            | sf::Style::Close
                                            | sf::Style::Titlebar;

static const sf::String Wndw_start_title{"SNAKE(TM)"};

//=========================================================

static sf::String Grass_bg_texture{"graph/textures/grass_bg.png"};
static sf::String Rabbit_texture{"graph/textures/bunny.png"};
static sf::String Rabbitsheet_texture{"graph/textures/bunnysheet.png"};

//=========================================================

void View_graph::run_loop()  
{
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (wndw.isOpen())
    {
        sf::Event event;
        while (wndw.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                wndw.close();
        }

        wndw.clear();
        wndw.draw(shape);
        wndw.display();
    }

    return;
}

//---------------------------------------------------------

Vector View_graph::get_winsize() const
{
    sf::Vector2u wndw_sz = wndw.getSize();
    return Vector{(ssize_t) wndw_sz.x, (ssize_t) wndw_sz.y};
}

//---------------------------------------------------------

View_graph::View_graph():
wndw(sf::VideoMode(Wndw_start_size.x, Wndw_start_size.y), 
                                      Wndw_start_title,
                                      Wndw_start_style)
{
    return;
}

//---------------------------------------------------------

View_graph::~View_graph()
{
    return;
}

//---------------------------------------------------------

void View_graph::load_textures()
{
    
}
