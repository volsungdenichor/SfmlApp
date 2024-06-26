#pragma once

#include "impl/shape_widget.hpp"
#include "impl/sprite_widget.hpp"
#include "impl/text_widget.hpp"
#include "widget_t.hpp"

using drawable_t = applier_t<sf::RenderTarget>;
using widget_modifier_t = applier_t<widget_t>;

inline auto operator|(widget_t lhs, widget_modifier_t rhs) -> widget_t
{
    rhs(lhs);
    return lhs;
}

inline auto operator|(widget_modifier_t lhs, widget_modifier_t rhs) -> widget_modifier_t
{
    return [=](widget_t& sh)
    {
        lhs(sh);
        rhs(sh);
    };
}

inline auto rect(float w, float h) -> widget_t
{
    return widget_t::create<shape_widget<sf::RectangleShape>>(sf::RectangleShape(sf::Vector2f(w, h)));
}

inline auto circle(float r) -> widget_t
{
    return widget_t::create<shape_widget<sf::CircleShape>>(sf::CircleShape(r));
}

inline auto polygon(const std::vector<vec_t>& v) -> widget_t
{
    sf::ConvexShape sh(v.size());
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        sh.setPoint(i, v[i]);
    }

    return widget_t::create<shape_widget<sf::ConvexShape>>(std::move(sh));
}

inline auto sprite(const texture_region_t& region) -> widget_t
{
    widget_t result = widget_t::create<sprite_widget>();
    result.set_texture(region);
    return result;
}

inline auto text(sf::String str, const sf::Font& font, unsigned int size)
{
    widget_t result = widget_t::create<text_widget>();
    result.set_text(set_value(std::move(str)));
    result.set_font(font);
    result.set_font_size(set_value(size));
    return result;
}

inline auto position(vec_t v) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_position(set_value(std::move(v))); };
}

inline auto fill(sf::Color color) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_fill_color(set_value(color)); };
}

inline auto outline(sf::Color color) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_outline_color(set_value(color)); };
}

inline auto outline_thickness(float v) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_outline_thickness(set_value(v)); };
}

inline auto rotate(float a) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_rotation(set_value(a)); };
}

inline auto texture(texture_region_t region) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_texture(region); };
}

inline auto bold() -> widget_modifier_t
{
    return [](widget_t& w) { w.set_font_style([](std::uint32_t& v) { v |= sf::Text::Bold; }); };
}

inline auto italic() -> widget_modifier_t
{
    return [](widget_t& w) { w.set_font_style([](std::uint32_t& v) { v |= sf::Text::Italic; }); };
}

template <class... Args>
auto all(Args&&... args) -> widget_modifier_t
{
    std::vector<widget_modifier_t> modifiers{ std::forward<Args>(args)... };
    return [=](widget_t& w)
    {
        for (const auto& m : modifiers)
        {
            m(w);
        }
    };
}
