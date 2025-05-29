#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <cstdint>
#include <functional>

#include "vec_t.hpp"

namespace foo
{

struct Style
{
    sf::Color fill_color = sf::Color::Black;
    sf::Color outline_color = sf::Color::White;
    float outline_thickness = 1.F;
};

struct TextStyle
{
    std::reference_wrapper<const sf::Font> font;
    unsigned int font_size = 16;
    float letter_spacing = 1.F;
    float line_spacing = 1.F;
    std::uint32_t style = sf::Text::Regular;
};

struct State
{
    Style style;
    TextStyle text_style;
    sf::RenderStates render_states;
};

struct Context
{
    sf::RenderTarget& target;
};

struct CanvasItem : public std::function<void(Context&, const State&)>
{
    using base_t = std::function<void(Context&, const State&)>;
    using base_t::base_t;
};

struct StateModifier : public std::function<void(State&)>
{
    using base_t = std::function<void(State&)>;
    using base_t::base_t;
};

struct StyleModifier : public std::function<void(Style&)>
{
    using base_t = std::function<void(Style&)>;
    using base_t::base_t;
};

struct TextStyleModifier : public std::function<void(TextStyle&)>
{
    using base_t = std::function<void(TextStyle&)>;
    using base_t::base_t;
};

struct RenderStatesModifier : public std::function<void(sf::RenderStates&)>
{
    using base_t = std::function<void(sf::RenderStates&)>;
    using base_t::base_t;
};

inline auto operator|(const StateModifier& lhs, const StateModifier& rhs) -> StateModifier
{
    return [=](State& state)
    {
        lhs(state);
        rhs(state);
    };
}

inline auto operator|(const CanvasItem& item, const StateModifier& modifier) -> CanvasItem
{
    return [=](Context& ctx, const State& state)
    {
        State new_state = state;
        modifier(new_state);
        item(ctx, new_state);
    };
}

inline void apply_style(sf::Shape& shape, const Style& style)
{
    shape.setFillColor(style.fill_color);
    shape.setOutlineColor(style.outline_color);
    shape.setOutlineThickness(style.outline_thickness);
}

inline auto modify_style(StyleModifier style_modifier) -> StateModifier
{
    return [=](State& state) { style_modifier(state.style); };
}

inline auto modify_text_style(TextStyleModifier text_style_modifier) -> StateModifier
{
    return [=](State& state) { text_style_modifier(state.text_style); };
}

inline auto modify_render_states(RenderStatesModifier render_states_modifier) -> StateModifier
{
    return [=](State& state) { render_states_modifier(state.render_states); };
}

inline auto text_style(std::uint32_t value) -> StateModifier
{
    return modify_text_style([=](TextStyle& text_style) { text_style.style = value; });
}

inline auto bold() -> StateModifier
{
    return modify_text_style([=](TextStyle& text_style) { text_style.style |= sf::Text::Bold; });
}

inline auto italic() -> StateModifier
{
    return modify_text_style([=](TextStyle& text_style) { text_style.style |= sf::Text::Italic; });
}

inline auto underlined() -> StateModifier
{
    return modify_text_style([=](TextStyle& text_style) { text_style.style |= sf::Text::Underlined; });
}

inline auto fill_color(const sf::Color& color) -> StateModifier
{
    return modify_style([=](Style& style) { style.fill_color = color; });
}

inline auto outline_color(const sf::Color& color) -> StateModifier
{
    return modify_style([=](Style& style) { style.outline_color = color; });
}

inline auto color(const sf::Color& color) -> StateModifier
{
    return fill_color(color) | outline_color(color);
}

inline auto outline_thickness(float value) -> StateModifier
{
    return modify_style([=](Style& style) { style.outline_thickness = value; });
}

inline auto font(const sf::Font& value) -> StateModifier
{
    return modify_text_style([&](TextStyle& text_style) { text_style.font = value; });
}

inline auto font_size(std::uint32_t value) -> StateModifier
{
    return modify_text_style([=](TextStyle& text_style) { text_style.font_size = value; });
}

inline auto blend(sf::BlendMode mode) -> StateModifier
{
    return modify_render_states([=](sf::RenderStates& render_states) { render_states.blendMode = mode; });
}

inline auto translate(const vec_t& v) -> StateModifier
{
    return modify_render_states([=](sf::RenderStates& render_states) { render_states.transform.translate(convert(v)); });
}

inline auto scale(const vec_t& v) -> StateModifier
{
    return modify_render_states([=](sf::RenderStates& render_states) { render_states.transform.scale(convert(v)); });
}

inline auto scale(const vec_t& v, const vec_t& pivot) -> StateModifier
{
    return translate(pivot) | scale(v) | translate(-pivot);
}

inline auto rotate(float a) -> StateModifier
{
    return modify_render_states([=](sf::RenderStates& render_states) { render_states.transform.rotate(sf::radians(a)); });
}

inline auto rotate(float a, const vec_t& pivot) -> StateModifier
{
    return translate(pivot) | rotate(a) | translate(-pivot);
}

inline auto group(std::vector<CanvasItem> items) -> CanvasItem
{
    return [=](Context& ctx, const State& state)
    {
        for (const auto& item : items)
        {
            item(ctx, state);
        }
    };
}

template <class... Tail>
auto group(CanvasItem head, Tail... tail) -> CanvasItem
{
    std::vector<CanvasItem> items{ std::move(head), CanvasItem(std::move(tail))... };
    return group(std::move(items));
}

inline auto text(const sf::String& str) -> CanvasItem
{
    return [=](Context& ctx, const State& state)
    {
        sf::Text shape{ state.text_style.font, str };
        shape.setFillColor(state.style.fill_color);
        shape.setOutlineColor(state.style.outline_color);
        shape.setOutlineThickness(state.style.outline_thickness);
        shape.setCharacterSize(state.text_style.font_size);
        shape.setLetterSpacing(state.text_style.letter_spacing);
        shape.setLineSpacing(state.text_style.line_spacing);
        shape.setStyle(state.text_style.style);
        ctx.target.draw(shape, state.render_states);
    };
}

inline auto rect(const vec_t& size) -> CanvasItem
{
    return [=](Context& ctx, const State& state)
    {
        sf::RectangleShape shape(convert(size));
        apply_style(shape, state.style);
        ctx.target.draw(shape, state.render_states);
    };
}

inline auto circle(float r) -> CanvasItem
{
    return [=](Context& ctx, const State& state)
    {
        sf::CircleShape shape(r);
        apply_style(shape, state.style);
        ctx.target.draw(shape, state.render_states);
    };
}

inline auto sprite(const sf::Texture& texture, const sf::IntRect& rect) -> CanvasItem
{
    return [&texture, rect](Context& ctx, const State& state)
    {
        sf::Sprite shape{ texture };
        shape.setTextureRect(rect);
        ctx.target.draw(shape, state.render_states);
    };
}

inline auto grid(const vec_t& size, const vec_t& dist) -> CanvasItem
{
    return [=](Context& ctx, const State& state)
    {
        for (int i = 0; i < size[0]; i += dist[0])
        {
            sf::Vertex line[2];
            line[0].position = sf::Vector2f(i, 0);
            line[0].color = state.style.outline_color;
            line[1].position = sf::Vector2f(i, size[1]);
            line[1].color = state.style.outline_color;
            ctx.target.draw(line, 2, sf::PrimitiveType::Lines, state.render_states);
        }
        for (int i = 0; i < size[1]; i += dist[1])
        {
            sf::Vertex line[2];
            line[0].position = sf::Vector2f(0, i);
            line[0].color = state.style.outline_color;
            line[1].position = sf::Vector2f(size[0], i);
            line[1].color = state.style.outline_color;
            ctx.target.draw(line, 2, sf::PrimitiveType::Lines, state.render_states);
        }
    };
}

inline auto triangle(const std::array<vec_t, 3>& vertices) -> CanvasItem
{
    return [=](Context& ctx, const State& state)
    {
        sf::VertexArray shape(sf::PrimitiveType::Triangles, 3);
        for (std::size_t i = 0; i < 3; ++i)
        {
            shape[i].position = convert(vertices[i]);
            shape[i].color = state.style.fill_color;
        }
        ctx.target.draw(shape, state.render_states);
    };
}

inline auto triangle(const vec_t& a, const vec_t& b, const vec_t& c) -> CanvasItem
{
    return triangle({ a, b, c });
}

inline auto polygon(const std::vector<vec_t>& vertices) -> CanvasItem
{
    return [=](Context& ctx, const State& state)
    {
        sf::ConvexShape shape{};
        shape.setPointCount(vertices.size());
        for (std::size_t i = 0; i < vertices.size(); ++i)
        {
            shape.setPoint(i, convert(vertices[i]));
        }
        apply_style(shape, state.style);
        ctx.target.draw(shape, state.render_states);
    };
}

}  // namespace foo
