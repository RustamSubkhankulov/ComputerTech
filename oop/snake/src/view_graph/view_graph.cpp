#include <iostream>
#include <stdexcept>
#include <climits>

//---------------------------------------------------------

#include "../../include/view_graph/view_graph.hpp"

//=========================================================

static const sf::Vector2u Wndw_start_size{1024, 768};

static const sf::Uint32 Wndw_start_style = sf::Style::Resize
                                            | sf::Style::Close
                                            | sf::Style::Titlebar;

static const sf::String Wndw_start_title{"SNAKE(TM)"};

//=========================================================

static sf::String Bg_texture_filename{"graph/textures/grass_bg.png"};
static sf::String Rabbit_texture_filename{"graph/textures/bunny.png"};
static sf::String Rabbitsheet_texture_filename{"graph/textures/bunnysheet.png"};
static sf::String Snakesheet_texture_filename{"graph/textures/snakesheet.png"};
static sf::String Snake_skin_texture_filename{"graph/textures/snake_skin.jpeg"};
static sf::String Snake_head_texture_filename{"graph/textures/snake_head.jpeg"};

//=========================================================

static void load_texture(sf::Texture& texture, const std::string& filename);

//=========================================================

void View_graph::run_loop()  
{
    sf::Clock clock;
    sf::Time prev = clock.getElapsedTime();
        
    int timeout = get_shortest_timeout();

    while (wndw.isOpen())
    {
        poll_events();  

        sf::Time cur = clock.getElapsedTime();
        sf::Time delta = cur - prev;
        sf::Int64 elapsed = delta.asMilliseconds();

        if ((int) elapsed > timeout)
        {
            bool upd = serve_subs_on_timer(elapsed);

            if (upd)
            {
                timeout = get_shortest_timeout();
                prev = clock.getElapsedTime();
            }
        }

        wndw.clear();
        draw();
        wndw.display();
    }

    return;
}

//---------------------------------------------------------

int View_graph::get_shortest_timeout()
{
    if (subs_on_timer.size() == 0)
        return 0;

    int min = INT_MAX;

    for (const auto& sub : subs_on_timer)
    {
        if ((int) sub.first < min)
            min = (int) sub.first;
    }

    return min;
}

//---------------------------------------------------------

struct Sub_remove_predicate
{
    bool operator() (on_timer_callback callback)
    {
        return (callback.first == 0);
    }
};

bool View_graph::serve_subs_on_timer(sf::Int64 elapsed)
{
    bool updated = false;

    unsigned elapsed_ms = (unsigned) elapsed;

    size_t count = subs_on_timer.size();

    for (auto& sub : subs_on_timer)
    {
        if (count == 0)
            break;

        if (elapsed_ms < sub.first)
            sub.first -= elapsed_ms;
        else 
        {
            updated = true;
            sub.first = 0;
            sub.second();
        }

        count -= 1;
    }

    auto remove = std::find_if(subs_on_timer.begin(), subs_on_timer.end(), Sub_remove_predicate());

    while ((remove) != subs_on_timer.end())
    {
        subs_on_timer.erase(remove);
        remove = std::find_if(subs_on_timer.begin(), subs_on_timer.end(), Sub_remove_predicate());
    }

    return updated;
}

//---------------------------------------------------------

void View_graph::serve_subs_on_key(sf::Keyboard::Key key)
{
    for (auto func : subs_on_key)
    {
        func((int) key);
    }
}

//---------------------------------------------------------

void View_graph::draw()
{
    draw_bg();
    draw_frame();

    Model* model = get_model();
    
    draw_rabbits(model);
    draw_snakes(model);
}

//---------------------------------------------------------

void View_graph::draw_lose_msg()
{

}

//---------------------------------------------------------

void View_graph::draw_frame()
{

}

//---------------------------------------------------------

void View_graph::draw_bg()
{
    sf::Sprite bg_sprite;
    bg_sprite.setTexture(bg_texture);

    sf::Vector2u size  = wndw.getSize();
    sf::Vector2i ssize = {(int) size.x, (int) size.y};
    bg_sprite.setTextureRect(sf::IntRect(sf::Vector2i{0, 0,}, ssize));

    wndw.draw(bg_sprite);
}

//---------------------------------------------------------

void View_graph::draw_rabbits(Model* model)
{
    sf::Sprite rabbit_sprite;
    rabbit_sprite.setTexture(rabbit_texture);

    sf::Vector2u txt_size = rabbit_texture.getSize();
    rabbit_sprite.setScale(sf::Vector2f{(float) Field_block_size / (float) txt_size.x,
                                        (float) Field_block_size / (float) txt_size.y});

    for (const Rabbit& rabbit : model->rabbits)
    {
        Coords rabbit_coords = rabbit.get_coords();
        rabbit_sprite.setPosition(sf::Vector2f{(float) (Field_block_size * rabbit_coords.x()), 
                                               (float) (Field_block_size * rabbit_coords.y())});

        wndw.draw(rabbit_sprite);
    }
}

//---------------------------------------------------------

void View_graph::draw_snakes(Model* model)
{
    sf::RectangleShape snake_body(sf::Vector2f((float) Field_block_size, 
                                               (float) Field_block_size));
    snake_body.setTexture(&snake_skin_texture);

    sf::RectangleShape snake_head(sf::Vector2f((float) Field_block_size, 
                                               (float) Field_block_size));
    snake_head.setTexture(&snake_head_texture);

    for (const Snake& snake : model->snakes)
    {
        Coords_list coords_list = snake.get_coords_list();

        unsigned head = (unsigned) coords_list.size() - 1;
        unsigned iter = 0;

        for (const Coords& coords : coords_list)
        {
            if (iter != head)
            {
                snake_body.setPosition(sf::Vector2f{(float) (Field_block_size * coords.x()),
                                                    (float) (Field_block_size * coords.y())});

                wndw.draw(snake_body);
            }
            else if (iter == head)
            {
                snake_head.setPosition(sf::Vector2f{(float) (Field_block_size * coords.x()),
                                                    (float) (Field_block_size * coords.y())});

                wndw.draw(snake_head);
            }

            iter++;
        }
    }
}

//---------------------------------------------------------

void View_graph::draw_results(Model* model)
{

}

//---------------------------------------------------------

void View_graph::poll_events()
{
    sf::Event event;

    bool event_avail = wndw.pollEvent(event);

    if (event_avail)
    {
        if (event.type == sf::Event::Closed)
            wndw.close();

        if (event.type == sf::Event::KeyPressed)
            serve_subs_on_key(event.key.code);
    }
}

//---------------------------------------------------------

Vector View_graph::get_winsize() const
{
    sf::Vector2u wndw_sz = wndw.getSize();
    // std::cerr << (ssize_t) wndw_sz.x / Field_block_size << ' ' << (ssize_t) wndw_sz.y / Field_block_size << std::endl;
    return Vector{(ssize_t) wndw_sz.x / Field_block_size, (ssize_t) wndw_sz.y / Field_block_size};
}

//---------------------------------------------------------

View_graph::View_graph():
wndw(sf::VideoMode(Wndw_start_size.x, Wndw_start_size.y), 
                                      Wndw_start_title,
                                      Wndw_start_style)
{
    load_textures();
    return;
}

//---------------------------------------------------------

View_graph::~View_graph()
{
    return;
}

//---------------------------------------------------------

static void load_texture(sf::Texture& texture, const std::string& filename)
{
    if (!texture.loadFromFile(filename))
        throw std::runtime_error{std::string{"Failed to load texture "} + filename};
}

//---------------------------------------------------------

void View_graph::load_textures()
{
    load_texture(bg_texture, Bg_texture_filename);
    load_texture(rabbit_texture, Rabbit_texture_filename);
    load_texture(rabbitsheet_texture, Rabbitsheet_texture_filename);
    load_texture(snakesheet_texture, Snakesheet_texture_filename);
    load_texture(snake_skin_texture, Snake_skin_texture_filename);
    load_texture(snake_head_texture, Snake_head_texture_filename);

    bg_texture.setSmooth(true);
    bg_texture.setRepeated(true);
}

//---------------------------------------------------------

void View_graph::field_sector_freed(const Coords& coords)
{
    Coords c = coords;
    c.normalize();
}
