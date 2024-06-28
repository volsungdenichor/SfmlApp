#pragma once

#include <SFML/Graphics.hpp>
#include <cassert>

struct sprite_widget : widget_impl
{
    sf::Sprite m_inner;

    explicit sprite_widget(sf::Sprite inner = {}) : m_inner(std::move(inner))
    {
    }

    std::unique_ptr<widget_impl> clone() const
    {
        return std::make_unique<sprite_widget>(m_inner);
    }

    void draw(sf::RenderTarget& window, sf::RenderStates states) const
    {
        window.draw(m_inner, states);
    }

    void set_position(const applier_t<vec_t>& applier) override
    {
        vec_t v(m_inner.getPosition());
        applier(v);
        m_inner.setPosition(v);
    }

    void set_scale(const applier_t<vec_t>& applier) override
    {
        vec_t v(m_inner.getScale());
        applier(v);
        m_inner.setScale(v);
    }

    void set_rotation(const applier_t<float>& applier) override
    {
        float r = m_inner.getRotation();
        applier(r);
        m_inner.setRotation(r);
    }

    void set_fill_color(const applier_t<sf::Color>& applier) override
    {
    }

    void set_outline_color(const applier_t<sf::Color>& applier) override
    {
    }

    void set_outline_thickness(const applier_t<float>& applier) override
    {
    }

    void set_texture(const std::optional<texture_region_t>& region) override
    {
        assert(region);
        m_inner.setTexture(region->texture.get());
        m_inner.setTextureRect(region->rect);
    }

    void set_text(const applier_t<sf::String>& applier) override
    {
    }

    void set_font(const sf::Font& font) override
    {
    }

    void set_font_size(const applier_t<unsigned int>& applier) override
    {
    }

    void set_line_spacing(const applier_t<float>& applier) override
    {
    }

    void set_letter_spacing(const applier_t<float>& applier) override
    {
    }

    void set_font_style(const applier_t<std::uint32_t>& applier) override
    {
    }
};