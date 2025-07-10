#pragma once

#include <cmath>

namespace anim
{

namespace ease
{

namespace detail
{

const inline float pi = std::asin(1.0F) * 2.F;
const inline float half_pi = pi / 2.F;

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

template <type Type>
struct ease_fn_t<Type, direction::in_out>
{
    float operator()(float t) const
    {
        static const ease_fn_t<Type, direction::in> in;
        static const ease_fn_t<Type, direction::out> out;

        return t < 0.5F  //
                   ? 0.5F * in(2.F * t)
                   : 0.5F * out(2.F * t - 1.F) + 0.5F;
    }
};

template <type Type>
struct ease_fn_t<Type, direction::out_in>
{
    float operator()(float t) const
    {
        static const ease_fn_t<Type, direction::out> out;
        static const ease_fn_t<Type, direction::in> in;

        return t < 0.5F  //
                   ? 0.5F * out(2.F * t)
                   : 0.5F * in(2.F * t - 1.F) + 0.5F;
    }
};

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
struct ease_fn_t<type::cubic, direction::out>
{
    float operator()(float t) const
    {
        return std::pow(t - 1.F, 3) + 1.F;
    }
};

template <>
struct ease_fn_t<type::quart, direction::in>
{
    float operator()(float t) const
    {
        return std::pow(t, 4);
    }
};

template <>
struct ease_fn_t<type::quart, direction::out>
{
    float operator()(float t) const
    {
        return -(std::pow(t - 1.F, 4) - 1.F);
    }
};

template <>
struct ease_fn_t<type::quint, direction::in>
{
    float operator()(float t) const
    {
        return std::pow(t, 5);
    }
};

template <>
struct ease_fn_t<type::quint, direction::out>
{
    float operator()(float t) const
    {
        return std::pow(t - 1.F, 5) + 1.F;
    }
};

template <>
struct ease_fn_t<type::sine, direction::in>
{
    float operator()(float t) const
    {
        return -std::cos(t * half_pi) + 1.F;
    }
};

template <>
struct ease_fn_t<type::sine, direction::out>
{
    float operator()(float t) const
    {
        return std::sin(t * half_pi);
    }
};

template <>
struct ease_fn_t<type::expo, direction::in>
{
    float operator()(float t) const
    {
        return t == 0.F ? 0.0F : std::pow(2.0F, 10 * (t - 1.F));
    }
};

template <>
struct ease_fn_t<type::expo, direction::out>
{
    float operator()(float t) const
    {
        return t == 1.F ? 1.F : -std::pow(2.0F, -10 * t) + 1.F;
    }
};

template <>
struct ease_fn_t<type::circ, direction::in>
{
    float operator()(float t) const
    {
        return -(std::sqrt(1.F - std::pow(t, 2)) - 1.F);
    }
};

template <>
struct ease_fn_t<type::circ, direction::out>
{
    float operator()(float t) const
    {
        return std::sqrt(1.F - std::pow(t - 1, 2));
    }
};

}  // namespace detail

constexpr inline auto none = detail::ease_fn_t<detail::type::linear, detail::direction::none>{};

constexpr inline auto quad_in = detail::ease_fn_t<detail::type::quad, detail::direction::in>{};
constexpr inline auto quad_out = detail::ease_fn_t<detail::type::quad, detail::direction::out>{};
constexpr inline auto quad_in_out = detail::ease_fn_t<detail::type::quad, detail::direction::in_out>{};
constexpr inline auto quad_out_in = detail::ease_fn_t<detail::type::quad, detail::direction::out_in>{};

constexpr inline auto cubic_in = detail::ease_fn_t<detail::type::cubic, detail::direction::in>{};
constexpr inline auto cubic_out = detail::ease_fn_t<detail::type::cubic, detail::direction::out>{};
constexpr inline auto cubic_in_out = detail::ease_fn_t<detail::type::cubic, detail::direction::in_out>{};
constexpr inline auto cubic_out_in = detail::ease_fn_t<detail::type::cubic, detail::direction::out_in>{};

constexpr inline auto quart_in = detail::ease_fn_t<detail::type::quart, detail::direction::in>{};
constexpr inline auto quart_out = detail::ease_fn_t<detail::type::quart, detail::direction::out>{};
constexpr inline auto quart_in_out = detail::ease_fn_t<detail::type::quart, detail::direction::in_out>{};
constexpr inline auto quart_out_in = detail::ease_fn_t<detail::type::quart, detail::direction::out_in>{};

constexpr inline auto quint_in = detail::ease_fn_t<detail::type::quint, detail::direction::in>{};
constexpr inline auto quint_out = detail::ease_fn_t<detail::type::quint, detail::direction::out>{};
constexpr inline auto quint_in_out = detail::ease_fn_t<detail::type::quint, detail::direction::in_out>{};
constexpr inline auto quint_out_in = detail::ease_fn_t<detail::type::quint, detail::direction::out_in>{};

constexpr inline auto sine_in = detail::ease_fn_t<detail::type::sine, detail::direction::in>{};
constexpr inline auto sine_out = detail::ease_fn_t<detail::type::sine, detail::direction::out>{};
constexpr inline auto sine_in_out = detail::ease_fn_t<detail::type::sine, detail::direction::in_out>{};
constexpr inline auto sine_out_in = detail::ease_fn_t<detail::type::sine, detail::direction::out_in>{};

constexpr inline auto expo_in = detail::ease_fn_t<detail::type::expo, detail::direction::in>{};
constexpr inline auto expo_out = detail::ease_fn_t<detail::type::expo, detail::direction::out>{};
constexpr inline auto expo_in_out = detail::ease_fn_t<detail::type::expo, detail::direction::in_out>{};
constexpr inline auto expo_out_in = detail::ease_fn_t<detail::type::expo, detail::direction::out_in>{};

constexpr inline auto circ_in = detail::ease_fn_t<detail::type::circ, detail::direction::in>{};
constexpr inline auto circ_out = detail::ease_fn_t<detail::type::circ, detail::direction::out>{};
constexpr inline auto circ_in_out = detail::ease_fn_t<detail::type::circ, detail::direction::in_out>{};
constexpr inline auto circ_out_in = detail::ease_fn_t<detail::type::circ, detail::direction::out_in>{};

}  // namespace ease

}  // namespace anim
