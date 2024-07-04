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
#include "event_handler_t.hpp"
#include "vec_t.hpp"

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

template <class T>
std::vector<T> repeat(T value, std::size_t count)
{
    return std::vector<T>(count, std::move(value));
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

namespace foo
{

struct style_t
{
    sf::Color fill_color = sf::Color::Black;
    sf::Color outline_color = sf::Color::White;
    float outline_thickness = 1.F;
};

struct text_style_t
{
    const sf::Font* font = nullptr;
    unsigned int font_size = 16;
    float letter_spacing = 1.F;
    float line_spacing = 1.F;
    std::uint32_t style = sf::Text::Regular;
};

struct state_t
{
    style_t style;
    text_style_t text_style;
    sf::RenderStates render_states;
};

struct context_t
{
    sf::RenderTarget* target;
};

using canvas_item = std::function<void(const state_t&, context_t&)>;
using state_modifier = action_t<state_t&>;

auto operator|(const state_modifier& lhs, const state_modifier& rhs) -> state_modifier
{
    return [=](state_t& state)
    {
        lhs(state);
        rhs(state);
    };
}

auto operator|(const canvas_item& item, const state_modifier& modifier) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        state_t new_state = state;
        modifier(new_state);
        item(new_state, ctx);
    };
}

void apply_style(sf::Shape& shape, const style_t& style)
{
    shape.setFillColor(style.fill_color);
    shape.setOutlineColor(style.outline_color);
    shape.setOutlineThickness(style.outline_thickness);
}

auto modify_state(const canvas_item& item, const action_t<state_t&>& op) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        state_t new_state = state;
        op(new_state);
        item(new_state, ctx);
    };
};

auto text_style(std::uint32_t value) -> state_modifier
{
    return [=](state_t& state) { state.text_style.style = value; };
}

auto bold() -> state_modifier
{
    return [=](state_t& state) { state.text_style.style |= sf::Text::Bold; };
}

auto italic() -> state_modifier
{
    return [=](state_t& state) { state.text_style.style |= sf::Text::Italic; };
}

auto underlined() -> state_modifier
{
    return [=](state_t& state) { state.text_style.style |= sf::Text::Underlined; };
}

auto blend(sf::BlendMode mode) -> state_modifier
{
    return [=](state_t& state) { state.render_states.blendMode = mode; };
}

auto fill_color(const sf::Color& color) -> state_modifier
{
    return [=](state_t& state) { state.style.fill_color = color; };
}

auto outline_color(const sf::Color& color) -> state_modifier
{
    return [=](state_t& state) { state.style.outline_color = color; };
}

auto outline_thickness(float value) -> state_modifier
{
    return [=](state_t& state) { state.style.outline_thickness = value; };
}

auto font(const sf::Font& value) -> state_modifier
{
    return [&](state_t& state) { state.text_style.font = &value; };
}

auto translate(const vec_t& v) -> state_modifier
{
    return [=](state_t& state) { state.render_states.transform.translate(v); };
}

auto scale(const vec_t& v) -> state_modifier
{
    return [=](state_t& state) { state.render_states.transform.scale(v); };
}

auto scale(const vec_t& v, const vec_t& pivot) -> state_modifier
{
    return translate(pivot) | scale(v) | translate(-pivot);
}

auto rotate(float a) -> state_modifier
{
    return [=](state_t& state) { state.render_states.transform.rotate(a); };
}

auto rotate(float a, const vec_t& pivot) -> state_modifier
{
    return translate(pivot) | rotate(a) | translate(-pivot);
}

template <class... Args>
auto group(Args&&... args) -> canvas_item
{
    std::vector<canvas_item> items{ canvas_item(args)... };
    return [=](const state_t& state, context_t& ctx)
    {
        for (const auto& item : items)
        {
            item(state, ctx);
        }
    };
}

auto text(const sf::String& str) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        sf::Text shape{};
        shape.setString(str);
        shape.setFillColor(state.style.fill_color);
        shape.setOutlineColor(state.style.outline_color);
        shape.setOutlineThickness(state.style.outline_thickness);
        if (state.text_style.font)
        {
            shape.setFont(*state.text_style.font);
        }
        shape.setCharacterSize(state.text_style.font_size);
        shape.setLetterSpacing(state.text_style.letter_spacing);
        shape.setLineSpacing(state.text_style.line_spacing);
        shape.setStyle(state.text_style.style);
        ctx.target->draw(shape, state.render_states);
    };
}

auto rect(const vec_t& size) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        sf::RectangleShape shape(size);
        apply_style(shape, state.style);
        ctx.target->draw(shape, state.render_states);
    };
}

auto circle(float r) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        sf::CircleShape shape(r);
        apply_style(shape, state.style);
        ctx.target->draw(shape, state.render_states);
    };
}

auto sprite(const sf::Texture& texture, const sf::IntRect& rect) -> canvas_item
{
    return [&texture, rect](const state_t& state, context_t& ctx)
    {
        sf::Sprite shape{};
        shape.setTexture(texture);
        shape.setTextureRect(rect);
        ctx.target->draw(shape, state.render_states);
    };
}

auto grid(const vec_t& size, const vec_t& dist) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        for (int i = 0; i < size[0]; i += dist[0])
        {
            sf::Vertex line[2];
            line[0].position = sf::Vector2f(i, 0);
            line[0].color = state.style.outline_color;
            line[1].position = sf::Vector2f(i, size[1]);
            line[1].color = state.style.outline_color;
            ctx.target->draw(line, 2, sf::PrimitiveType::Lines, state.render_states);
        }
        for (int i = 0; i < size[1]; i += dist[1])
        {
            sf::Vertex line[2];
            line[0].position = sf::Vector2f(0, i);
            line[0].color = state.style.outline_color;
            line[1].position = sf::Vector2f(size[0], i);
            line[1].color = state.style.outline_color;
            ctx.target->draw(line, 2, sf::PrimitiveType::Lines, state.render_states);
        }
    };
}

auto triangle(const std::array<vec_t, 3>& vertices) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        sf::VertexArray shape(sf::PrimitiveType::Triangles, 3);
        for (std::size_t i = 0; i < 3; ++i)
        {
            shape[i].position = vertices[i];
            shape[i].color = state.style.fill_color;
        }
        ctx.target->draw(shape, state.render_states);
    };
}

auto triangle(const vec_t& a, const vec_t& b, const vec_t& c) -> canvas_item
{
    return triangle({ a, b, c });
}

auto polygon(const std::vector<vec_t>& vertices) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        sf::ConvexShape shape{};
        shape.setPointCount(vertices.size());
        for (std::size_t i = 0; i < vertices.size(); ++i)
        {
            shape.setPoint(i, vertices[i]);
        }
        apply_style(shape, state.style);
        ctx.target->draw(shape, state.render_states);
    };
}

auto array(const std::vector<canvas_item>& items, const vec_t& dist) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        for (std::size_t i = 0; i < items.size(); ++i)
        {
            (items[i] | translate(dist * i))(state, ctx);
        }
    };
}

}  // namespace foo

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
                    foo::array(
                        repeat(
                            foo::array(
                                { foo::circle(10) | foo::fill_color(sf::Color::Cyan),
                                  foo::circle(10) | foo::fill_color(sf::Color::Magenta),
                                  foo::circle(10) | foo::fill_color(sf::Color::Yellow) },
                                { 100, 0 })
                                | foo::outline_color(sf::Color::Black),
                            4),
                        { 0, 50 })
                        | foo::translate({ 500, 0 }),
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