#pragma once

#include <SFML/System.hpp>
#include <array>
#include <ferrugo/alg/matrix.hpp>

using vec_t = ferrugo::alg::vector_2d<float>;
using box_t = vec_t;

inline sf::Vector2f convert(const vec_t& v)
{
    return sf::Vector2f(v.x(), v.y());
}

inline vec_t convert(const sf::Vector2f& v)
{
    return vec_t{ v.x, v.y };
}