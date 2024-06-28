#pragma once

#include <SFML/Graphics.hpp>

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

    void set_geometry(
        const applier_t<vec_t>& position, const applier_t<vec_t>& scale, const applier_t<float>& rotation) override
    {
        {
            vec_t v(m_inner.getPosition());
            position(v);
            m_inner.setPosition(v);
        }
        {
            vec_t v(m_inner.getScale());
            scale(v);
            m_inner.setScale(v);
        }
        {
            float r = m_inner.getRotation();
            rotation(r);
            m_inner.setRotation(r);
        }
    }

    void set_style(
        const applier_t<sf::Color>& fill_color,
        const applier_t<sf::Color>& outline_color,
        const applier_t<float>& outline_thickness) override
    {
    }

    void set_texture(const sf::Texture* texture, const sf::IntRect& rect)
    {
        m_inner.setTexture(*texture);
        m_inner.setTextureRect(rect);
    }

    void set_text(const applier_t<sf::String>& text, const sf::Font& font, const applier_t<unsigned int>& size) override
    {
    }
};