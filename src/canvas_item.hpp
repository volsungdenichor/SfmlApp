#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <cstdint>
#include <functional>

#include "vec_t.hpp"

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
using state_modifier = std::function<void(state_t&)>;

inline auto operator|(const state_modifier& lhs, const state_modifier& rhs) -> state_modifier
{
    return [=](state_t& state)
    {
        lhs(state);
        rhs(state);
    };
}

inline auto operator|(const canvas_item& item, const state_modifier& modifier) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        state_t new_state = state;
        modifier(new_state);
        item(new_state, ctx);
    };
}

inline void apply_style(sf::Shape& shape, const style_t& style)
{
    shape.setFillColor(style.fill_color);
    shape.setOutlineColor(style.outline_color);
    shape.setOutlineThickness(style.outline_thickness);
}

inline auto text_style(std::uint32_t value) -> state_modifier
{
    return [=](state_t& state) { state.text_style.style = value; };
}

inline auto bold() -> state_modifier
{
    return [=](state_t& state) { state.text_style.style |= sf::Text::Bold; };
}

inline auto italic() -> state_modifier
{
    return [=](state_t& state) { state.text_style.style |= sf::Text::Italic; };
}

inline auto underlined() -> state_modifier
{
    return [=](state_t& state) { state.text_style.style |= sf::Text::Underlined; };
}

inline auto blend(sf::BlendMode mode) -> state_modifier
{
    return [=](state_t& state) { state.render_states.blendMode = mode; };
}

inline auto fill_color(const sf::Color& color) -> state_modifier
{
    return [=](state_t& state) { state.style.fill_color = color; };
}

inline auto outline_color(const sf::Color& color) -> state_modifier
{
    return [=](state_t& state) { state.style.outline_color = color; };
}

inline auto color(const sf::Color& color) -> state_modifier
{
    return fill_color(color) | outline_color(color);
}

inline auto outline_thickness(float value) -> state_modifier
{
    return [=](state_t& state) { state.style.outline_thickness = value; };
}

inline auto font(const sf::Font& value) -> state_modifier
{
    return [&](state_t& state) { state.text_style.font = &value; };
}

inline auto translate(const vec_t& v) -> state_modifier
{
    return [=](state_t& state) { state.render_states.transform.translate(convert(v)); };
}

inline auto scale(const vec_t& v) -> state_modifier
{
    return [=](state_t& state) { state.render_states.transform.scale(convert(v)); };
}

inline auto scale(const vec_t& v, const vec_t& pivot) -> state_modifier
{
    return translate(pivot) | scale(v) | translate(-pivot);
}

inline auto rotate(float a) -> state_modifier
{
    return [=](state_t& state) { state.render_states.transform.rotate(a); };
}

inline auto rotate(float a, const vec_t& pivot) -> state_modifier
{
    return translate(pivot) | rotate(a) | translate(-pivot);
}

inline auto group(std::vector<canvas_item> items) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        for (const auto& item : items)
        {
            item(state, ctx);
        }
    };
}

template <class... Tail>
auto group(canvas_item head, Tail... tail) -> canvas_item
{
    std::vector<canvas_item> items{ std::move(head), canvas_item(std::move(tail))... };
    return group(std::move(items));
}

inline auto text(const sf::String& str) -> canvas_item
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

inline auto rect(const vec_t& size) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        sf::RectangleShape shape(convert(size));
        apply_style(shape, state.style);
        ctx.target->draw(shape, state.render_states);
    };
}

inline auto circle(float r) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        sf::CircleShape shape(r);
        apply_style(shape, state.style);
        ctx.target->draw(shape, state.render_states);
    };
}

inline auto sprite(const sf::Texture& texture, const sf::IntRect& rect) -> canvas_item
{
    return [&texture, rect](const state_t& state, context_t& ctx)
    {
        sf::Sprite shape{};
        shape.setTexture(texture);
        shape.setTextureRect(rect);
        ctx.target->draw(shape, state.render_states);
    };
}

inline auto grid(const vec_t& size, const vec_t& dist) -> canvas_item
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

inline auto triangle(const std::array<vec_t, 3>& vertices) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        sf::VertexArray shape(sf::PrimitiveType::Triangles, 3);
        for (std::size_t i = 0; i < 3; ++i)
        {
            shape[i].position = convert(vertices[i]);
            shape[i].color = state.style.fill_color;
        }
        ctx.target->draw(shape, state.render_states);
    };
}

inline auto triangle(const vec_t& a, const vec_t& b, const vec_t& c) -> canvas_item
{
    return triangle({ a, b, c });
}

inline auto polygon(const std::vector<vec_t>& vertices) -> canvas_item
{
    return [=](const state_t& state, context_t& ctx)
    {
        sf::ConvexShape shape{};
        shape.setPointCount(vertices.size());
        for (std::size_t i = 0; i < vertices.size(); ++i)
        {
            shape.setPoint(i, convert(vertices[i]));
        }
        apply_style(shape, state.style);
        ctx.target->draw(shape, state.render_states);
    };
}

}  // namespace foo