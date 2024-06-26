#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <memory>
#include <optional>

#include "../defs.hpp"
#include "../vec_t.hpp"

struct texture_region_t
{
    std::reference_wrapper<const sf::Texture> texture;
    sf::IntRect rect;
};
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
    virtual void set_texture(const std::optional<texture_region_t>& region) = 0;
    virtual void set_text(const applier_t<sf::String>& applier) = 0;
    virtual void set_font(const sf::Font& font) = 0;
    virtual void set_font_size(const applier_t<unsigned int>& applier) = 0;
    virtual void set_line_spacing(const applier_t<float>& applier) = 0;
    virtual void set_letter_spacing(const applier_t<float>& applier) = 0;
    virtual void set_font_style(const applier_t<std::uint32_t>& applier) = 0;
};
