#pragma once

#include <cmath>
#include <numeric>

#include "ease.hpp"

using time_point_t = float;
using duration_t = float;

inline time_point_t wrap(time_point_t time, duration_t duration, time_point_t inflection_point = {})
{
    return time > duration ? inflection_point + std::fmod(time, duration - inflection_point) : time;
}

template <class T>
struct animation_impl
{
    virtual ~animation_impl() = default;

    virtual duration_t duration() const = 0;
    virtual T value(time_point_t t) const = 0;
    virtual T start_value() const = 0;
    virtual T end_value() const = 0;

    T wrapped_value(time_point_t t, time_point_t inflection_point = {}) const
    {
        return value(wrap(t, duration(), inflection_point));
    }

    float duration_ratio(time_point_t t) const
    {
        return t / duration();
    }
};

template <class T>
using animation_ptr = std::shared_ptr<animation_impl<T>>;

template <class T>
struct animation
{
    animation_ptr<T> m_impl;

    explicit animation(animation_ptr<T> impl) : m_impl(std::move(impl))
    {
    }

    duration_t duration() const
    {
        return m_impl->duration();
    }

    T value(time_point_t t) const
    {
        return m_impl->value(t);
    }

    T operator()(time_point_t t) const
    {
        return value(t);
    }

    T start_value() const
    {
        return m_impl->start_value();
    }

    T end_value() const
    {
        return m_impl->end_value();
    }

    T wrapped_value(time_point_t t, time_point_t inflection_point = {}) const
    {
        return m_impl->wrapped_value(t, inflection_point);
    }
};

struct reverse_fn
{
    template <class T>
    class impl : public animation_impl<T>
    {
    public:
        explicit impl(animation_ptr<T> inner) : m_inner(std::move(inner))
        {
        }

        duration_t duration() const override
        {
            return m_inner->duration();
        }

        T value(time_point_t t) const override
        {
            return m_inner->value(duration() - t);
        }

        T start_value() const override
        {
            return m_inner->end_value();
        }

        T end_value() const override
        {
            return m_inner->start_value();
        }

    private:
        animation_ptr<T> m_inner;
    };

    template <class T>
    auto operator()(animation<T> inner) const -> animation<T>
    {
        return animation<T>(std::make_shared<impl<T>>(inner.m_impl));
    }
};

struct repeat_fn
{
    template <class T>
    class impl : public animation_impl<T>
    {
    public:
        explicit impl(animation_ptr<T> inner, float count, time_point_t inflection_point)
            : m_inner(std::move(inner))
            , m_count(count)
            , m_inflection_point(inflection_point)
        {
        }

        duration_t duration() const override
        {
            return m_inner->duration() * m_count;
        }

        T value(time_point_t t) const override
        {
            return m_inner->wrapped_value(t, m_inflection_point);
        }

        T start_value() const override
        {
            return m_inner->start_value();
        }

        T end_value() const override
        {
            return value(duration());
        }

    private:
        animation_ptr<T> m_inner;
        float m_count;
        time_point_t m_inflection_point;
    };

    template <class T>
    auto operator()(animation<T> inner, float count, time_point_t inflection_point = {}) const -> animation<T>
    {
        return animation<T>(std::make_shared<impl<T>>(inner.m_impl, count, inflection_point));
    }
};

struct constant_fn
{
    template <class T>
    class impl : public animation_impl<T>
    {
    public:
        explicit impl(T value, duration_t duration) : m_value(value), m_duration(duration)
        {
        }

        duration_t duration() const override
        {
            return m_duration;
        }

        T value(time_point_t t) const override
        {
            return m_value;
        }

        T start_value() const override
        {
            return m_value;
        }

        T end_value() const override
        {
            return m_value;
        }

    private:
        T m_value;
        duration_t m_duration;
    };
    template <class T>
    auto operator()(T value, duration_t duration) const -> animation<T>
    {
        return animation<T>(std::make_shared<impl<T>>(value, duration));
    }
};

using ease_function = std::function<float(float)>;

template <class R, class T>
auto lerp(R ratio, T a, T b) -> std::invoke_result_t<std::multiplies<>, R, T>
{
    return ((R(1) - ratio) * a) + (ratio * b);
}

struct gradual_fn
{
    template <class T>
    class impl : public animation_impl<T>
    {
    public:
        impl(duration_t duration, T start_value, T end_value, ease_function ease)
            : m_duration(duration)
            , m_start_value(start_value)
            , m_end_value(end_value)
            , m_ease(std::move(ease))
        {
        }

        duration_t duration() const
        {
            return m_duration;
        }

        T value(time_point_t t) const override
        {
            return static_cast<T>(lerp(m_ease(this->duration_ratio(t)), m_start_value, m_end_value));
        }

        T start_value() const override
        {
            return m_start_value;
        }

        T end_value() const override
        {
            return m_end_value;
        }

    private:
        duration_t m_duration;
        T m_start_value;
        T m_end_value;
        ease_function m_ease;
    };

    template <class T>
    auto operator()(T start_value, T end_value, duration_t duration, ease_function ease) const -> animation<T>
    {
        return animation<T>(std::make_shared<impl<T>>(duration, start_value, end_value, std::move(ease)));
    }
};

struct sequence_fn
{
    template <class T>
    class impl : public animation_impl<T>
    {
    public:
        explicit impl(std::vector<animation_ptr<T>> ptrs) : m_ptrs(std::move(ptrs)), m_duration(0.F)
        {
            m_duration = std::accumulate(
                std::begin(m_ptrs),
                std::end(m_ptrs),
                duration_t{},
                [](duration_t total, const animation_ptr<T>& item) { return total + item->duration(); });
        }

        duration_t duration() const
        {
            return m_duration;
        }

        T value(time_point_t t) const override
        {
            if (t < 0.F)
            {
                return start_value();
            }
            else if (t >= duration())
            {
                return end_value();
            }

            for (const auto& ptr : m_ptrs)
            {
                if (t > ptr->duration())
                {
                    t -= ptr->duration();
                }
                else
                {
                    return ptr->value(t);
                }
            }

            return end_value();
        }

        T start_value() const override
        {
            return m_ptrs.front()->start_value();
        }

        T end_value() const override
        {
            return m_ptrs.back()->end_value();
        }

    private:
        std::vector<animation_ptr<T>> m_ptrs;
        duration_t m_duration;
    };

    template <class T, class... Tail>
    auto operator()(animation<T> head, Tail... tail) const -> animation<T>
    {
        std::vector<animation_ptr<T>> ptrs{ head.m_impl, tail.m_impl... };
        return animation<T>(std::make_shared<impl<T>>(std::move(ptrs)));
    }
};

static constexpr inline auto reverse = reverse_fn{};
static constexpr inline auto repeat = repeat_fn{};
static constexpr inline auto constant = constant_fn{};
static constexpr inline auto gradual = gradual_fn{};
static constexpr inline auto sequence = sequence_fn{};
