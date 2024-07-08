#pragma once

#include <SFML/System.hpp>
#include <array>
#include <ferrugo/geo/matrix.hpp>

using vec_t = ferrugo::geo::vector_2d<float>;

inline sf::Vector2f convert(const vec_t& v)
{
    return sf::Vector2f(v.x(), v.y());
}

inline vec_t convert(const sf::Vector2f& v)
{
    return vec_t{ v.x, v.y };
}