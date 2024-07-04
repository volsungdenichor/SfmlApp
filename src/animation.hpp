#pragma once

#include <cmath>
#include <numeric>

#include "ease.hpp"

namespace anim
{

using time_point_t = float;
using duration_t = float;

using ease_function = std::function<float(float)>;

inline time_point_t wrap(time_point_t time, duration_t duration, time_point_t inflection_point = {})
{
    return time > duration ? inflection_point + std::fmod(time, duration - inflection_point) : time;
}

inline float lerp(float ratio, float a, float b)
{
    return ((1.F - ratio) * a) + (ratio * b);
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
            return m_inner->wrapped_value(this->duration());
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

struct ping_pong_fn
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
            const time_point_t t_ = std::fmod(t, m_inner->duration());
            return (static_cast<int>(t / m_inner->duration()) % 2 == 0) ? m_inner->value(t_)
                                                                        : m_inner->value(m_inner->duration() - t_);
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

struct slice_fn
{
    template <class T>
    class impl : public animation_impl<T>
    {
    public:
        explicit impl(animation_ptr<T> inner, time_point_t start, time_point_t end)
            : m_inner(std::move(inner))
            , m_start(start)
            , m_end(end)
        {
        }

        duration_t duration() const override
        {
            return m_end - m_start;
        }

        T value(time_point_t t) const override
        {
            return m_inner->value(clamp(t));
        }

        T start_value() const override
        {
            return value(0.F);
        }

        T end_value() const override
        {
            return value(duration());
        }

    private:
        time_point_t clamp(time_point_t t) const
        {
            return std::min(std::min(m_start + t, m_inner->duration()), m_end);
        }

        animation_ptr<T> m_inner;
        time_point_t m_start;
        time_point_t m_end;
    };

    template <class T>
    auto operator()(animation<T> inner, time_point_t start, time_point_t end) const -> animation<T>
    {
        return animation<T>(std::make_shared<impl<T>>(inner.m_impl, start, end));
    }
};

struct rescale_fn
{
    template <class T>
    class impl : public animation_impl<T>
    {
    public:
        explicit impl(animation_ptr<T> inner, duration_t duration) : m_inner(std::move(inner)), m_duration(duration)
        {
        }

        duration_t duration() const override
        {
            return m_duration;
        }

        T value(time_point_t t) const override
        {
            return m_inner->value(t * m_inner->duration() / m_duration);
        }

        T start_value() const override
        {
            return value(0.F);
        }

        T end_value() const override
        {
            return value(duration());
        }

    private:
        animation_ptr<T> m_inner;
        duration_t m_duration;
    };

    template <class T>
    auto operator()(animation<T> inner, duration_t duration) const -> animation<T>
    {
        return animation<T>(std::make_shared<impl<T>>(inner.m_impl, duration));
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

    template <class T>
    auto operator()(std::vector<animation<T>> vect) const -> animation<T>
    {
        std::vector<animation_ptr<T>> ptrs;
        ptrs.reserve(vect.size());
        std::transform(
            std::begin(vect), std::end(vect), std::back_inserter(ptrs), [](const animation<T>& a) { return a.m_impl; });
        return animation<T>(std::make_shared<impl<T>>(std::move(ptrs)));
    }
};

static constexpr inline auto reverse = reverse_fn{};
static constexpr inline auto repeat = repeat_fn{};
static constexpr inline auto ping_pong = ping_pong_fn{};
static constexpr inline auto constant = constant_fn{};
static constexpr inline auto gradual = gradual_fn{};
static constexpr inline auto slice = slice_fn{};
static constexpr inline auto rescale = rescale_fn{};
static constexpr inline auto sequence = sequence_fn{};

}  // namespace anim