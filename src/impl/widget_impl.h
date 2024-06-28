#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "../defs.hpp"
#include "../vec_t.hpp"

struct widget_impl
{
    using ptr = std::unique_ptr<widget_impl>;

    virtual ~widget_impl() = default;
    virtual std::unique_ptr<widget_impl> clone() const = 0;
    virtual void draw(sf::RenderTarget& window, sf::RenderStates states) const = 0;
    virtual void set_geometry(
        const applier_t<vec_t>& position, const applier_t<vec_t>& scale, const applier_t<float>& rotation)
        = 0;

    virtual void set_style(
        const applier_t<sf::Color>& fill_color,
        const applier_t<sf::Color>& outline_color,
        const applier_t<float>& outline_thickness)
        = 0;

    virtual void set_texture(const sf::Texture* texture, const sf::IntRect& rect) = 0;

    virtual void set_text(const applier_t<sf::String>& text, const sf::Font& font, const applier_t<unsigned int>& size) = 0;
};
