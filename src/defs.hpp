#pragma once

struct do_nothing_fn
{
    template <class... Args>
    constexpr void operator()(Args&&...) const
    {
    }
};

static constexpr inline auto do_nothing = do_nothing_fn{};

template <class... Args>
using action_t = std::function<void(Args...)>;

template <class T>
using applier_t = action_t<T&>;

template <class T>
auto set_value(T v) -> applier_t<T>
{
    return [=](T& out) { out = v; };
}
