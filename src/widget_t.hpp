#pragma once

#include "widget_impl.h"

struct widget_t
{
    widget_impl::ptr m_ptr;

    explicit widget_t(widget_impl::ptr ptr) : m_ptr(std::move(ptr))
    {
    }

    widget_t(widget_t&&) = default;

    widget_t(const widget_t& other) : widget_t(m_ptr->clone())
    {
    }

    template <class T, class... Args>
    static widget_t create(Args&&... args)
    {
        return widget_t(std::make_unique<T>(std::forward<Args>(args)...));
    }

    widget_t& operator=(widget_t other)
    {
        std::swap(m_ptr, other.m_ptr);
        return *this;
    }

    void draw(sf::RenderTarget& window, sf::RenderStates states) const
    {
        m_ptr->draw(window, std::move(states));
    }

    void operator()(sf::RenderTarget& window) const
    {
        draw(window, sf::RenderStates::Default);
    }

    void set_geometry(const applier_t<vec_t>& position, const applier_t<vec_t>& scale, const applier_t<float>& rotation)
    {
        m_ptr->set_geometry(position, scale, rotation);
    }

    void set_style(
        const applier_t<sf::Color>& fill_color,
        const applier_t<sf::Color>& outline_color,
        const applier_t<float>& outline_thickness)
    {
        m_ptr->set_style(fill_color, outline_color, outline_thickness);
    }

    void set_texture(const sf::Texture* texture, const sf::IntRect& rect)
    {
        m_ptr->set_texture(texture, rect);
    }

    void set_text(const applier_t<sf::String>& text, const sf::Font& font, const applier_t<unsigned int>& size)
    {
        m_ptr->set_text(text, font, size);
    }
};