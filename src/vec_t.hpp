#pragma once

#include <SFML/System.hpp>
#include <array>

struct vec_t : std::array<float, 2>
{
    using base_t = std::array<float, 2>;

    vec_t(float x, float y) : base_t{ { x, y } }
    {
    }

    vec_t(const sf::Vector2f& v) : vec_t(v.x, v.y)
    {
    }

    vec_t() : vec_t(0.F, 0.F)
    {
    }

    float x() const
    {
        return (*this)[0];
    }

    float y() const
    {
        return (*this)[1];
    }

    operator sf::Vector2f() const
    {
        return sf::Vector2f(x(), y());
    }

    friend vec_t operator+(const vec_t& item)
    {
        return item;
    }

    friend vec_t operator-(vec_t item)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            item[i] = -item[i];
        }
        return item;
    }

    friend vec_t& operator*=(vec_t& lhs, float rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] *= rhs;
        }
        return lhs;
    }

    friend vec_t operator*(vec_t lhs, float rhs)
    {
        return lhs *= rhs;
    }

    friend vec_t& operator/=(vec_t& lhs, float rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] /= rhs;
        }
        return lhs;
    }

    friend vec_t operator/(vec_t lhs, float rhs)
    {
        return lhs /= rhs;
    }

    friend vec_t operator*(float lhs, const vec_t& rhs)
    {
        return rhs * lhs;
    }

    friend vec_t& operator+=(vec_t& lhs, const vec_t& rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] += rhs[i];
        }
        return lhs;
    }

    friend vec_t operator+(vec_t lhs, const vec_t& rhs)
    {
        return lhs += rhs;
    }

    friend vec_t& operator-=(vec_t& lhs, const vec_t& rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] -= rhs[i];
        }
        return lhs;
    }

    friend vec_t operator-(vec_t lhs, const vec_t& rhs)
    {
        return lhs -= rhs;
    }
};