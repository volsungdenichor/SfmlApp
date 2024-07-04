#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>

#include "animation.hpp"
#include "app_runner.hpp"
#include "event_handler_t.hpp"
#include "vec_t.hpp"

template <class T>
T create(const action_t<T&>& init)
{
    T result{};
    init(result);
    return result;
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

auto blend(sf::BlendMode mode) -> state_modifier
{
    return [=](state_t& state) { state.render_states.blendMode = mode; };
}

auto blend(sf::BlendMode mode, const canvas_item& item) -> canvas_item
{
    return modify_state(item, blend(mode));
}

auto fill_color(const sf::Color& color) -> state_modifier
{
    return [=](state_t& state) { state.style.fill_color = color; };
}

auto fill_color(const sf::Color& color, const canvas_item& item) -> canvas_item
{
    return modify_state(item, fill_color(color));
}

auto outline_color(const sf::Color& color) -> state_modifier
{
    return [=](state_t& state) { state.style.outline_color = color; };
}

auto outline_color(const sf::Color& color, const canvas_item& item) -> canvas_item
{
    return modify_state(item, outline_color(color));
}

auto outline_thickness(float value) -> state_modifier
{
    return [=](state_t& state) { state.style.outline_thickness = value; };
}

auto outline_thickness(float value, const canvas_item& item) -> canvas_item
{
    return modify_state(item, outline_thickness(value));
}

auto font(const sf::Font& value) -> state_modifier
{
    return [&](state_t& state) { state.text_style.font = &value; };
}

auto font(const sf::Font& value, const canvas_item& item) -> canvas_item
{
    return modify_state(item, font(value));
}

auto translate(const vec_t& v) -> state_modifier
{
    return [=](state_t& state) { state.render_states.transform.translate(v); };
}

auto translate(const vec_t& v, const canvas_item& item) -> canvas_item
{
    return modify_state(item, translate(v));
}

auto scale(const vec_t& v) -> state_modifier
{
    return [=](state_t& state) { state.render_states.transform.scale(v); };
}

auto scale(const vec_t& v, const canvas_item& item)
{
    return modify_state(item, scale(v));
}

auto rotate(float a) -> state_modifier
{
    return [=](state_t& state) { state.render_states.transform.rotate(a); };
}

auto rotate(float a, const canvas_item& item) -> canvas_item
{
    return modify_state(item, rotate(a));
}

template <class... Args>
auto layer(Args&&... args) -> canvas_item
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

    float rotation_speed = 10.F;

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
    };

    const sf::Texture txt = load_texture(R"(/mnt/d/Users/Krzysiek/Pictures/conan.bmp)");
    const sf::Font arial = load_font(R"(/mnt/c/Windows/Fonts/arial.ttf)");
    const sf::Font verdana = load_font(R"(/mnt/c/Windows/Fonts/verdana.ttf)");

    float angle = 0.F;

    run_app(
        window,
        event_handler,
        [&](float dt) { angle += dt * rotation_speed; },
        render_scene(
            [&](const vec_t& size, float fps) -> foo::canvas_item
            {
                return foo::layer(
                    foo::grid(size, { 100.F, 100.F })  //
                        | foo::outline_color(sf::Color(64, 0, 0)),
                    foo::sprite(txt, { 0, 0, 200, 200 })  //
                        | foo::rotate(45.F - angle)       //
                        | foo::translate({ 300, 300 }),
                    foo::triangle({ 10, 0 }, { 20, 20 }, { 0, 20 })  //
                        | foo::translate({ 100, 500 })               //
                        | foo::fill_color(sf::Color::Red),
                    foo::text(std::to_string(fps))       //
                        | foo::translate({ 1000, 600 })  //
                        | foo::outline_thickness(0.F)    //
                        | foo::fill_color(sf::Color::White),
                    foo::layer(
                        foo::circle(50),
                        foo::layer(
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