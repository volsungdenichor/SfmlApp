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

    void set_fill_color(const applier_t<sf::Color>& applier)
    {
        sf::Color v = m_inner.getFillColor();
        applier(v);
        m_inner.setFillColor(v);
    }

    void set_outline_color(const applier_t<sf::Color>& applier)
    {
        sf::Color v = m_inner.getOutlineColor();
        applier(v);
        m_inner.setOutlineColor(v);
    }

    void set_outline_thickness(const applier_t<float>& applier)
    {
        float v = m_inner.getOutlineThickness();
        applier(v);
        m_inner.setOutlineThickness(v);
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
