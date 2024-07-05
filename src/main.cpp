#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

#include "animation.hpp"
#include "app_runner.hpp"
#include "canvas_item.hpp"
#include "event_handler_t.hpp"
#include "vec_t.hpp"

template <class Range>
using iterator_t = decltype(std::begin(std::declval<Range>()));

template <class Iter>
using iter_reference_t = typename std::iterator_traits<Iter>::reference;

template <class Func, class Range, class Res = std::invoke_result_t<Func, iter_reference_t<iterator_t<Range>>>>
auto bind(Func&& func, Range&& range) -> Res
{
    Res result;
    for (auto&& item : range)
    {
        auto res = std::invoke(func, std::forward<decltype(item)>(item));
        result.insert(std::end(result), std::begin(res), std::end(res));
    }
    return result;
}

template <class... Args>
std::string str(Args&&... args)
{
    std::stringstream ss;
    (ss << ... << args);
    return ss.str();
}

template <class T>
T create(const action_t<T&>& init)
{
    T result{};
    init(result);
    return result;
}

template <class T, class... Tail>
std::vector<T> vec(T head, Tail&&... tail)
{
    return std::vector<T>{ std::move(head), std::forward<Tail>(tail)... };
}

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

action_t<sf::RenderWindow&, float> render_scene(
    const std::function<foo::canvas_item(const vec_t&, float)>& scene_builder, const sf::Font* default_font)
{
    return [=](sf::RenderWindow& w, float fps)
    {
        const auto size = vec_t{ float(w.getSize().x), float(w.getSize().y) };
        foo::context_t ctx{ &w };
        foo::state_t state{};
        state.text_style.font = default_font;
        const foo::canvas_item scene = scene_builder(size, fps);
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

    const sf::Texture txt = load_texture(R"(/mnt/d/Users/Krzysiek/Pictures/conan.bmp)");
    const sf::Font arial = load_font(R"(/mnt/c/Windows/Fonts/arial.ttf)");
    const sf::Font verdana = load_font(R"(/mnt/c/Windows/Fonts/verdana.ttf)");

    run_app(
        window,
        event_handler,
        [&](float dt) { angle += dt * rotation_speed * acceleration; },
        render_scene(
            [&](const vec_t& size, float fps) -> foo::canvas_item
            {
                return foo::group(
                    foo::grid(size, { 100.F, 100.F })  //
                        | foo::outline_color(sf::Color(64, 0, 0)),
                    foo::polygon({ { 0, 0 },   { 20, 0 },  { 30, 10 }, { 50, 10 }, { 60, 0 },  { 80, 0 },  { 80, 20 },
                                   { 70, 30 }, { 70, 50 }, { 80, 60 }, { 80, 80 }, { 60, 80 }, { 50, 70 }, { 30, 70 },
                                   { 20, 80 }, { 0, 80 },  { 0, 60 },  { 10, 50 }, { 10, 30 }, { 0, 20 } })
                        | foo::rotate(angle * 3.F, { 40, 40 })    //
                        | foo::translate({ 800, 400 })            //
                        | foo::outline_color(sf::Color::Magenta)  //
                        | foo::fill_color(sf::Color::Magenta),
                    foo::sprite(txt, { 0, 0, 200, 200 })  //
                        | foo::rotate(45.F - angle)       //
                        | foo::translate({ 300, 300 }),
                    foo::triangle({ 10, 0 }, { 20, 20 }, { 0, 20 })  //
                        | foo::translate({ 100, 500 })               //
                        | foo::fill_color(sf::Color::Red),
                    foo::map(
                        foo::distribute({ 100.F, 0.F }),
                        { foo::circle(10) | foo::fill_color(sf::Color::Cyan),
                          foo::circle(10) | foo::fill_color(sf::Color::Magenta),
                          foo::circle(10) | foo::fill_color(sf::Color::Yellow) })
                        | foo::translate({ 0, 700 }),
                    foo::repeat(foo::distribute({ 100.F, 0.F }), foo::circle(20), 10) | foo::translate({ 0, 800 }),
                    foo::text(str("[x", acceleration, "] fps=", std::fixed, std::setprecision(1), fps))  //
                        | foo::bold()                                                                    //
                        | foo::translate({ 1000, 600 })                                                  //
                        | foo::outline_thickness(0.F)                                                    //
                        | foo::fill_color(sf::Color::White),
                    foo::group(
                        foo::circle(50) | foo::fill_color(sf::Color(255, 0, 0)),
                        foo::group(
                            foo::circle(50.F)  //
                                | foo::translate({ 200, 0 }),
                            foo::rect({ 20.F, 20.F })                   //
                                | foo::outline_thickness(2.F)           //
                                | foo::outline_color(sf::Color::Green)  //
                                | foo::fill_color(sf::Color::Blue),
                            foo::rect({ 10.F, 5.F })               //
                                | foo::fill_color(sf::Color::Red)  //
                                | foo::translate({ 100, 50 })      //
                                | foo::rotate(45.F))               //
                            | foo::translate({ 200, 200 })         //
                            | foo::rotate(angle))
                        | foo::translate({ 200, 0 })  //
                        | foo::scale({ 1.F, 2.F }));
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