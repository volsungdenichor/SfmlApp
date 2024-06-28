#pragma once

#include <SFML/Graphics.hpp>

#include "widget_impl.h"

struct text_widget : widget_impl
{
    sf::Text m_inner;

    explicit text_widget(sf::Text inner = {}) : m_inner(std::move(inner))
    {
    }

    std::unique_ptr<widget_impl> clone() const
    {
        return std::make_unique<text_widget>(m_inner);
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

    void set_style(
        const applier_t<sf::Color>& fill_color,
        const applier_t<sf::Color>& outline_color,
        const applier_t<float>& outline_thickness) override
    {
        {
            sf::Color v = m_inner.getFillColor();
            fill_color(v);
            m_inner.setFillColor(v);
        }
        {
            sf::Color v = m_inner.getOutlineColor();
            outline_color(v);
            m_inner.setOutlineColor(v);
        }
        {
            float v = m_inner.getOutlineThickness();
            outline_thickness(v);
            m_inner.setOutlineThickness(v);
        }
    }

    void set_texture(const sf::Texture* texture, const sf::IntRect& rect)
    {
    }

    void set_text(const applier_t<sf::String>& text, const sf::Font& font, const applier_t<unsigned int>& size) override
    {
        {
            sf::String v = m_inner.getString();
            text(v);
            m_inner.setString(v);
        }
        m_inner.setFont(font);
        {
            auto v = m_inner.getCharacterSize();
            size(v);
            m_inner.setCharacterSize(v);
        }
    }
};
