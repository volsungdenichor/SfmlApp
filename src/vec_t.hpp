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

template <class T>
inline vec_t convert(const sf::Vector2<T>& v)
{
    return vec_t{ static_cast<float>(v.x), static_cast<float>(v.y) };
}
