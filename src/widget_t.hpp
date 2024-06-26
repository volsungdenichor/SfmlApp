#pragma once

#include "impl/widget_impl.h"

struct widget_t
{
    widget_impl::ptr m_impl;

    explicit widget_t(widget_impl::ptr impl) : m_impl(std::move(impl))
    {
    }

    widget_t(widget_t&&) = default;

    widget_t(const widget_t& other) : widget_t(m_impl->clone())
    {
    }

    template <class T, class... Args>
    static widget_t create(Args&&... args)
    {
        return widget_t(std::make_unique<T>(std::forward<Args>(args)...));
    }

    widget_t& operator=(widget_t other)
    {
        std::swap(m_impl, other.m_impl);
        return *this;
    }

    void draw(sf::RenderTarget& window, sf::RenderStates states) const
    {
        m_impl->draw(window, std::move(states));
    }

    void operator()(sf::RenderTarget& window) const
    {
        draw(window, sf::RenderStates::Default);
    }

    void set_position(const applier_t<vec_t>& applier)
    {
        m_impl->set_position(applier);
    }

    void set_scale(const applier_t<vec_t>& applier)
    {
        m_impl->set_scale(applier);
    }

    void set_rotation(const applier_t<float>& applier)
    {
        m_impl->set_rotation(applier);
    }

    void set_fill_color(const applier_t<sf::Color>& applier)
    {
        m_impl->set_fill_color(applier);
    }

    void set_outline_color(const applier_t<sf::Color>& applier)
    {
        m_impl->set_outline_color(applier);
    }

    void set_outline_thickness(const applier_t<float>& applier)
    {
        m_impl->set_outline_thickness(applier);
    }

    void set_texture(const texture_region_t& region)
    {
        m_impl->set_texture(region);
    }

    void set_text(const applier_t<sf::String>& applier)
    {
        m_impl->set_text(applier);
    }

    void set_font(const sf::Font& font)
    {
        m_impl->set_font(font);
    }

    void set_font_size(const applier_t<unsigned int>& applier)
    {
        m_impl->set_font_size(applier);
    }

    void set_line_spacing(const applier_t<float>& applier)
    {
        m_impl->set_line_spacing(applier);
    }

    void set_letter_spacing(const applier_t<float>& applier)
    {
        m_impl->set_letter_spacing(applier);
    }

    void set_font_style(const applier_t<std::uint32_t>& applier)
    {
        m_impl->set_font_style(applier);
    }
};