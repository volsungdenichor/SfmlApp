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
    virtual void set_position(const applier_t<vec_t>& applier) = 0;
    virtual void set_scale(const applier_t<vec_t>& applier) = 0;
    virtual void set_rotation(const applier_t<float>& applier) = 0;
    virtual void set_fill_color(const applier_t<sf::Color>& applier) = 0;
    virtual void set_outline_color(const applier_t<sf::Color>& applier) = 0;
    virtual void set_outline_thickness(const applier_t<float>& applier) = 0;
    virtual void set_texture(const sf::Texture* texture, const sf::IntRect& rect) = 0;

    virtual void set_text(const applier_t<sf::String>& text, const sf::Font& font, const applier_t<unsigned int>& size) = 0;
};
