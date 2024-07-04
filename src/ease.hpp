#pragma once

#include <cmath>

namespace anim
{

namespace ease
{

namespace detail
{

enum class type
{
    linear,
    quad,
    cubic,
    quart,
    quint,
    sine,
    expo,
    circ,
    back,
    elastic,
    bounce,
};

enum class direction
{
    none,
    in,
    out,
    in_out,
    out_in,
};

template <type Type, direction C>
struct ease_fn_t;

template <direction C>
struct ease_fn_t<type::linear, C>
{
    float operator()(float t) const
    {
        return t;
    }
};

template <>
struct ease_fn_t<type::quad, direction::in>
{
    float operator()(float t) const
    {
        return std::pow(t, 2);
    }
};

template <>
struct ease_fn_t<type::quad, direction::out>
{
    float operator()(float t) const
    {
        return -t * (t - 2.F);
    }
};

template <>
struct ease_fn_t<type::cubic, direction::in>
{
    float operator()(float t) const
    {
        return std::pow(t, 3);
    }
};

template <>
struct ease_fn_t<type::quad, direction::in_out>
{
    float operator()(float t) const
    {
        t *= 2;

        if (t < 1)
        {
            return 0.5F * std::pow(t, 2);
        }

        t -= 1;
        return -0.5F * (t * (t - 2.F) - 1.F);
    }
};

template <>
struct ease_fn_t<type::cubic, direction::out>
{
    float operator()(float t) const
    {
        return std::pow(t - 1, 3) + 1;
    }
};

template <>
struct ease_fn_t<type::cubic, direction::in_out>
{
    float operator()(float t) const
    {
        t *= 2;
        return t < 1 ? 0.5F * std::pow(t, 3) : 0.5F * (std::pow(t - 2, 3) + 2);
    }
};

}  // namespace detail

static const inline auto none = detail::ease_fn_t<detail::type::linear, detail::direction::none>{};
static const inline auto quad_in = detail::ease_fn_t<detail::type::quad, detail::direction::in>{};
static const inline auto quad_out = detail::ease_fn_t<detail::type::quad, detail::direction::out>{};
static const inline auto quad_in_out = detail::ease_fn_t<detail::type::quad, detail::direction::in_out>{};
static const inline auto cubic_in = detail::ease_fn_t<detail::type::cubic, detail::direction::in>{};
static const inline auto cubic_out = detail::ease_fn_t<detail::type::cubic, detail::direction::out>{};
static const inline auto cubic_in_out = detail::ease_fn_t<detail::type::cubic, detail::direction::in_out>{};

}  // namespace ease

}  // namespace anim
