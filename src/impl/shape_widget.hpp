#pragma once

#include "widget_impl.h"

template <class S>
struct shape_widget : widget_impl
{
    S m_inner;

    explicit shape_widget(S inner) : m_inner(std::move(inner))
    {
    }

    std::unique_ptr<widget_impl> clone() const override
    {
        return std::make_unique<shape_widget<S>>(m_inner);
    }

    void draw(sf::RenderTarget& window, sf::RenderStates states) const override
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

    void set_texture(const sf::Texture* texture, const sf::IntRect& rect) override
    {
        m_inner.setTexture(texture);
        m_inner.setTextureRect(rect);
    }

    void set_text(const applier_t<sf::String>& text, const sf::Font& font, const applier_t<unsigned int>& size) override
    {
    }
};
