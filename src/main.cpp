#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>
#include <ferrugo/core/functional.hpp>
#include <ferrugo/seq/seq.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

#include "animation.hpp"
#include "app_runner.hpp"
#include "canvas_item.hpp"
#include "event_handler.hpp"
#include "vec_t.hpp"

sf::Texture load_texture(const std::string& path)
{
    sf::Texture result{};
    if (!result.loadFromFile(path))
    {
        throw std::runtime_error{ "Unable to load texture from " + path };
    }
    return result;
}

sf::Font load_font(const std::string& path)
{
    sf::Font result{};
    if (!result.loadFromFile(path))
    {
        throw std::runtime_error{ "Unable to load font from " + path };
    }
    return result;
}

render_fn render_scene(
    const std::function<foo::canvas_item_t(const vec_t&, float)>& scene_builder, const sf::Font* default_font)
{
    return [=](sf::RenderWindow& w, float fps)
    {
        const auto size = vec_t{ float(w.getSize().x), float(w.getSize().y) };
        foo::context_t ctx{ &w };
        foo::state_t state{};
        state.text_style.font = default_font;
        const foo::canvas_item_t scene = scene_builder(size, fps);
        scene(state, ctx);
    };
}

void run()
{
    auto window = sf::RenderWindow{ { 1920u, 1080u }, "CMake SFML Project" };

    int acceleration = 1;
    float rotation_speed = 30.F;
    float angle = 0.F;

    event_handler_t event_handler{};
    event_handler.on_close = [](sf::RenderWindow& w) { w.close(); };
    event_handler.on_key_pressed = [&](sf::RenderWindow& w, const sf::Event::KeyEvent& e)
    {
        if (e.code == sf::Keyboard::Escape)
        {
            w.close();
        }
        if (e.code == sf::Keyboard::Space)
        {
            rotation_speed = -rotation_speed;
        }
        if (e.code == sf::Keyboard::Tab)
        {
            if (acceleration == 16)
            {
                acceleration = 1;
            }
            else
            {
                acceleration *= 2;
            }
        }
    };

    const sf::Font arial = load_font(R"(/mnt/c/Windows/Fonts/arial.ttf)");
    const sf::Font verdana = load_font(R"(/mnt/c/Windows/Fonts/verdana.ttf)");

    using namespace ferrugo;

    run_app(
        window,
        event_handler,
        [&](float dt) { angle += dt * rotation_speed * acceleration; },
        render_scene(
            [&](const vec_t& size, float fps) -> foo::canvas_item_t
            {
                return foo::group(
                           foo::grid(size, vec_t{ 100.F, 100.F }) | foo::outline_color(sf::Color(64, 0, 0)),
                           foo::group(seq::init(
                               15,
                               [](int x) -> foo::canvas_item_t
                               {
                                   auto shape = x % 2 == 0 ? foo::circle(50.F) : foo::rect(vec_t{ 100.F, 100.F });
                                   return shape                                        //
                                          | foo::translate(vec_t{ 150.F * x, 500.F })  //
                                          | foo::color(sf::Color(200, 120, 140));
                               })),
                           foo::group(
                               seq::repeat(foo::circle(50.F))  //
                               |= seq::take(3)                 //
                               |= seq::transform_i(
                                   [](int i, const foo::canvas_item_t& item) -> foo::canvas_item_t
                                   {
                                       return item                                   //
                                              | foo::translate(vec_t{ 0, 125 } * i)  //
                                              | foo::color(sf::Color::Red);
                                   })))
                       | foo::rotate(angle, vec_t{ 960.F, 540.F });
            },
            &verdana),
        0.01F);
}

int main()
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}