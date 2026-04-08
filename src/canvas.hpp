#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <array>
#include <cstdint>
#include <functional>
#include <zx/mat.hpp>

template <class T>
sf::Vector2<T> convert(const zx::mat::vector_t<T, 2>& v)
{
    return sf::Vector2<T>(v[0], v[1]);
}

template <class U, class T>
sf::Vector2<U> convert(const zx::mat::vector_t<T, 2>& v)
{
    return sf::Vector2<U>(static_cast<U>(v[0]), static_cast<U>(v[1]));
}

template <class T>
zx::mat::vector_t<T, 2> convert(const sf::Vector2<T>& v)
{
    return zx::mat::vector_t<T, 2>{ v.x, v.y };
}

template <class U, class T>
zx::mat::vector_t<U, 2> convert_as(const sf::Vector2<T>& v)
{
    return zx::mat::vector_t<U, 2>{ static_cast<U>(v.x), static_cast<U>(v.y) };
}

namespace canvas
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

struct DrawOp : public std::function<void(Context&, const State&)>
{
    using base_t = std::function<void(Context&, const State&)>;
    using base_t::base_t;
};

struct StateModifier
{
    using Modifier = std::function<void(State&)>;

    template <class M>
    StateModifier(M&& modifier) : m_modifiers{ std::forward<M>(modifier) }
    {
    }

    StateModifier(const StateModifier&) = default;
    StateModifier(StateModifier&&) noexcept = default;

    void operator()(State& state) const
    {
        for (const auto& modifier : m_modifiers)
        {
            modifier(state);
        }
    }

    friend auto operator|(StateModifier lhs, StateModifier rhs) -> StateModifier
    {
        lhs.m_modifiers.reserve(lhs.m_modifiers.size() + rhs.m_modifiers.size());
        lhs.m_modifiers.insert(
            lhs.m_modifiers.end(),
            std::make_move_iterator(rhs.m_modifiers.begin()),
            std::make_move_iterator(rhs.m_modifiers.end()));
        return lhs;
    }

private:
    std::vector<Modifier> m_modifiers;
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

inline auto operator|(DrawOp item, StateModifier modifier) -> DrawOp
{
    return [item = std::move(item), modifier = std::move(modifier)](Context& ctx, const State& state)
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
    return [sm = std::move(style_modifier)](State& state) { sm(state.style); };
}

inline auto modify_text_style(TextStyleModifier text_style_modifier) -> StateModifier
{
    return [tsm = std::move(text_style_modifier)](State& state) { tsm(state.text_style); };
}

inline auto modify_render_states(RenderStatesModifier render_states_modifier) -> StateModifier
{
    return [rsm = std::move(render_states_modifier)](State& state) { rsm(state.render_states); };
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

inline auto translate(const zx::mat::vector_t<float, 2>& v) -> StateModifier
{
    return modify_render_states([=](sf::RenderStates& render_states) { render_states.transform.translate(convert(v)); });
}

inline auto scale(const zx::mat::vector_t<float, 2>& v) -> StateModifier
{
    return modify_render_states([=](sf::RenderStates& render_states) { render_states.transform.scale(convert(v)); });
}

inline auto scale(const zx::mat::vector_t<float, 2>& v, const zx::mat::vector_t<float, 2>& pivot) -> StateModifier
{
    return translate(pivot) | scale(v) | translate(-pivot);
}

inline auto rotate(float a) -> StateModifier
{
    return modify_render_states([=](sf::RenderStates& render_states) { render_states.transform.rotate(sf::radians(a)); });
}

inline auto rotate(float a, const zx::mat::vector_t<float, 2>& pivot) -> StateModifier
{
    return translate(pivot) | rotate(a) | translate(-pivot);
}

inline auto empty_item() -> DrawOp
{
    return [](Context&, const State&) {};
}

inline auto group(std::vector<DrawOp> items) -> DrawOp
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
auto group(DrawOp head, Tail... tail) -> DrawOp
{
    std::vector<DrawOp> items{ std::move(head), DrawOp(std::move(tail))... };
    return group(std::move(items));
}

template <class Func, class Range>
auto transform(Func&& func, Range&& range) -> DrawOp
{
    std::vector<DrawOp> items;
    for (auto&& item : range)
    {
        items.push_back(std::invoke(func, item));
    }
    return group(std::move(items));
}

template <class Func, class Range>
auto transform_maybe(Func&& func, Range&& range) -> DrawOp
{
    std::vector<DrawOp> items;
    for (auto&& item : range)
    {
        std::optional<DrawOp> res = std::invoke(func, item);
        if (res)
        {
            items.push_back(*std::move(res));
        }
    }
    return group(std::move(items));
}

inline auto text(const sf::String& str) -> DrawOp
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

inline auto rect(const zx::mat::vector_t<float, 2>& size) -> DrawOp
{
    return [=](Context& ctx, const State& state)
    {
        sf::RectangleShape shape(convert(size));
        apply_style(shape, state.style);
        ctx.target.draw(shape, state.render_states);
    };
}

inline auto circle(float r) -> DrawOp
{
    return [=](Context& ctx, const State& state)
    {
        sf::CircleShape shape(r);
        apply_style(shape, state.style);
        ctx.target.draw(shape, state.render_states);
    };
}

inline auto circle(const zx::mat::spherical_shape_t<float, 2>& c) -> DrawOp
{
    return circle(c.radius) | translate(c.center - zx::mat::vector_t<float, 2>{ c.radius, c.radius });
}

inline auto point(const zx::mat::vector_t<float, 2>& p, float radius = 3.F) -> canvas::DrawOp
{
    return circle(zx::mat::spherical_shape_t<float, 2>{ p, radius });
}

inline auto sprite(const sf::Texture& texture, const sf::IntRect& rect) -> DrawOp
{
    return [&texture, rect](Context& ctx, const State& state)
    {
        sf::Sprite shape{ texture };
        shape.setTextureRect(rect);
        ctx.target.draw(shape, state.render_states);
    };
}

inline auto grid(const zx::mat::vector_t<float, 2>& size, const zx::mat::vector_t<float, 2>& dist) -> DrawOp
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

inline auto triangle(const std::array<zx::mat::vector_t<float, 2>, 3>& vertices) -> DrawOp
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

inline auto triangle(
    const zx::mat::vector_t<float, 2>& a, const zx::mat::vector_t<float, 2>& b, const zx::mat::vector_t<float, 2>& c)
    -> DrawOp
{
    return triangle({ a, b, c });
}

inline auto polygon(const std::vector<zx::mat::vector_t<float, 2>>& vertices) -> DrawOp
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

inline auto shape(const zx::mat::spherical_shape_t<float, 2>& item) -> DrawOp
{
    return circle(item);
}

inline auto shape(const zx::mat::triangle_t<float, 2>& item) -> DrawOp
{
    return triangle(item);
}

inline auto shape(const zx::mat::segment_t<float, 2>& item) -> DrawOp
{
    return [=](Context& ctx, const State& state)
    {
        sf::VertexArray shape(sf::PrimitiveType::Lines, 2);
        for (std::size_t i = 0; i < 2; ++i)
        {
            shape[i].position = convert(item[i]);
            shape[i].color = state.style.outline_color;
        }
        ctx.target.draw(shape, state.render_states);
    };
}

inline auto shape(const zx::mat::vector_t<float, 2>& item) -> DrawOp
{
    return point(item);
}

}  // namespace canvas
