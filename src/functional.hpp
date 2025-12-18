#pragma once

#include <functional>
#include <type_traits>

namespace detail
{

struct apply_fn
{
    template <class Func>
    struct impl_t
    {
        Func m_func;

        template <class T>
        auto operator()(T& item) const -> T&
        {
            std::invoke(m_func, item);
            return item;
        }

        template <class T>
        constexpr friend auto operator|(T& item, const impl_t& impl) -> T&
        {
            return impl(std::move(item));
        }
    };

    template <class Func>
    constexpr auto operator()(Func&& func) const -> impl_t<std::decay_t<Func>>
    {
        return { std::forward<Func>(func) };
    }
};
struct with_fn
{
    template <class Func>
    struct impl_t
    {
        Func m_func;

        template <class T>
        auto operator()(T item) const -> T
        {
            std::invoke(m_func, item);
            return item;
        }

        template <class T>
        constexpr friend auto operator|(T item, const impl_t& impl) -> T
        {
            return impl(std::move(item));
        }
    };

    template <class Func>
    constexpr auto operator()(Func&& func) const -> impl_t<std::decay_t<Func>>
    {
        return { std::forward<Func>(func) };
    }
};

}  // namespace detail

static constexpr auto apply = detail::apply_fn{};
static constexpr auto with = detail::with_fn{};

template <class T>
constexpr auto create(std::function<void(T&)> func) -> T
{
    return T{} | with(std::move(func));
}
