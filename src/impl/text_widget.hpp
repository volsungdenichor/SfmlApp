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

    void set_fill_color(const applier_t<sf::Color>& applier) override
    {
        sf::Color v = m_inner.getFillColor();
        applier(v);
        m_inner.setFillColor(v);
    }

    void set_outline_color(const applier_t<sf::Color>& applier) override
    {
        sf::Color v = m_inner.getOutlineColor();
        applier(v);
        m_inner.setOutlineColor(v);
    }

    void set_outline_thickness(const applier_t<float>& applier) override
    {
        float v = m_inner.getOutlineThickness();
        applier(v);
        m_inner.setOutlineThickness(v);
    }

    void set_texture(const std::optional<texture_region_t>& region) override
    {
    }

    void set_text(const applier_t<sf::String>& applier) override
    {
        sf::String v = m_inner.getString();
        applier(v);
        m_inner.setString(v);
    }

    void set_font(const sf::Font& font) override
    {
        m_inner.setFont(font);
    }

    void set_font_size(const applier_t<unsigned int>& applier) override
    {
        unsigned int v = m_inner.getCharacterSize();
        applier(v);
        m_inner.setCharacterSize(v);
    }

    void set_line_spacing(const applier_t<float>& applier) override
    {
        float v = m_inner.getLineSpacing();
        applier(v);
        m_inner.setLineSpacing(v);
    }

    void set_letter_spacing(const applier_t<float>& applier) override
    {
        float v = m_inner.getLetterSpacing();
        applier(v);
        m_inner.setLetterSpacing(v);
    }

    void set_font_style(const applier_t<std::uint32_t>& applier) override
    {
        auto v = m_inner.getStyle();
        applier(v);
        m_inner.setStyle(v);
    }
};
