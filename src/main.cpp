#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
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

using canvas_item = std::function<state_t(const state_t&, context_t&)>;

auto fill_color(const sf::Color& color, const canvas_item& item) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        state_t new_state = state;
        new_state.style.fill_color = color;
        item(new_state, ctx);
        return state;
    };
}

auto outline_color(const sf::Color& color, const canvas_item& item) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        state_t new_state = state;
        new_state.style.outline_color = color;
        item(new_state, ctx);
        return state;
    };
}

auto outline_thickness(float value, const canvas_item& item) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        state_t new_state = state;
        new_state.style.outline_thickness = value;
        item(new_state, ctx);
        return state;
    };
}

auto font(const sf::Font& value, const canvas_item& item) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        state_t new_state = state;
        new_state.text_style.font = &value;
        item(new_state, ctx);
        return state;
    };
}

void apply_style(sf::Shape& shape, const style_t& style)
{
    shape.setFillColor(style.fill_color);
    shape.setOutlineColor(style.outline_color);
    shape.setOutlineThickness(style.outline_thickness);
}

auto text(const sf::String& str)
{
    return [=](const state_t& state, context_t& ctx) -> state_t
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
        ctx.target->draw(shape, state.render_states);
        return state;
    };
}

auto rect(const vec_t& size) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        sf::RectangleShape shape(size);
        apply_style(shape, state.style);
        ctx.target->draw(shape, state.render_states);
        return state;
    };
}

auto circle(float r) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        sf::CircleShape shape(r);
        apply_style(shape, state.style);
        ctx.target->draw(shape, state.render_states);
        return state;
    };
}

auto translate(const vec_t& v, const canvas_item& item) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        state_t new_state = state;
        new_state.render_states.transform.translate(v);
        item(new_state, ctx);
        return state;
    };
}

auto scale(const vec_t& v, const canvas_item& item) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        state_t new_state = state;
        new_state.render_states.transform.scale(v);
        item(new_state, ctx);
        return state;
    };
}

auto rotate(float a, const canvas_item& item) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        state_t new_state = state;
        new_state.render_states.transform.rotate(a);
        item(new_state, ctx);
        return state;
    };
}

template <class... Args>
auto layer(Args&&... args) -> canvas_item
{
    std::vector<canvas_item> items{ canvas_item(args)... };
    return [=](const state_t& state, context_t& ctx) -> state_t
    {
        state_t new_state = state;
        for (const auto& item : items)
        {
            new_state = item(new_state, ctx);
        }
        return state;
    };
}

auto grid(const vec_t& size, const vec_t& dist) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx) -> state_t
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
        return state;
    };
}

}  // namespace foo

void run()
{
    auto window = sf::RenderWindow{ { 1920u, 1080u }, "CMake SFML Project" };

    event_handler_t event_handler{};
    event_handler.on_close = [](sf::RenderWindow& w) { w.close(); };
    event_handler.on_key_pressed = [&](sf::RenderWindow& w, const sf::Event::KeyEvent& e)
    {
        if (e.code == sf::Keyboard::Escape)
        {
            w.close();
        }
    };

    const sf::Font arial = load_font(R"(/mnt/c/Windows/Fonts/arial.ttf)");
    const sf::Font verdana = load_font(R"(/mnt/c/Windows/Fonts/verdana.ttf)");

    float angle = 0.F;

    run_app(
        window,
        event_handler,
        [&](float dt) { angle += dt * 10.F; },
        [&](sf::RenderWindow& w, float fps)
        {
            const auto obj = foo::layer(
                foo::outline_color(
                    sf::Color(64, 0, 0), foo::grid({ float(w.getSize().x), float(w.getSize().y) }, { 100.F, 100.F })),
                foo::translate(
                    { 1000, 600 },
                    foo::outline_thickness(0.F, foo::fill_color(sf::Color::White, foo::text(std::to_string(fps))))),
                foo::translate(
                    { 200, 0 },
                    foo::scale(
                        { 1.F, 2.0F },
                        foo::layer(
                            foo::circle(50),
                            foo::rotate(
                                angle,
                                foo::translate(
                                    { 200, 200 },
                                    foo::layer(
                                        foo::translate({ 200, 0 }, foo::circle(50.F)),
                                        foo::outline_color(
                                            sf::Color::Red, foo::fill_color(sf::Color::Green, foo::rect({ 20.F, 20.F }))),
                                        foo::fill_color(
                                            sf::Color::Red,
                                            foo::translate({ 100, 50 }, foo::rotate(45.F, foo::rect({ 10.F, 5.F })))))))))));

            foo::context_t ctx{ &w };
            foo::state_t state{};
            state.text_style.font = &verdana;
            obj(state, ctx);
        },
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