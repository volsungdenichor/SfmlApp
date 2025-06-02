#pragma once

#include <SFML/Graphics.hpp>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <typeindex>

using fps_t = float;       // 1/s
using duration_t = float;  // s

struct TickEvent
{
    duration_t elapsed;
};

struct InitEvent
{
};

template <class Model, class Msg>
struct App
{
    using UpdateResult = std::tuple<Model, std::optional<Msg>>;
    using RenderFn = std::function<void(const Model&, sf::RenderWindow&)>;
    using UpdateFn = std::function<UpdateResult(const Model&, const Msg&)>;
    using HandleMsgFn = std::function<void(sf::RenderWindow&, const Msg&)>;

    sf::RenderWindow& m_window;
    Model m_model_state;
    RenderFn render = {};
    UpdateFn update;
    HandleMsgFn on_msg;
    std::deque<Msg> m_msg_queue;
    duration_t m_frame_duration = duration_t{ 0.01 };

    void handle_event(UpdateResult event_result)
    {
        auto [new_model_state, maybe_msg] = std::move(event_result);
        m_model_state = std::move(new_model_state);
        if (maybe_msg)
        {
            m_msg_queue.push_back(*maybe_msg);
        }
    }

    template <class Head, class... Tail>
    void handle_event(const sf::Event& event)
    {
        if (const auto e = event.getIf<Head>())
        {
            publish_event(*e);
        }

        if constexpr (sizeof...(Tail) > 0)
        {
            handle_event<Tail...>(event);
        }
    }

    void run()
    {
        sf::Clock clock;
        duration_t time_since_last_update = 0.F;

        publish_event(InitEvent{});

        while (m_window.isOpen())
        {
            const duration_t elapsed = clock.restart().asSeconds();
            time_since_last_update += elapsed;

            const fps_t fps = 1.0F / elapsed;

            while (time_since_last_update > m_frame_duration)
            {
                time_since_last_update -= m_frame_duration;

                while (const std::optional<sf::Event> event = m_window.pollEvent())
                {
                    if (event->is<sf::Event::Closed>())
                    {
                        m_window.close();
                    }

                    handle_event<
                        sf::Event::Resized,
                        sf::Event::FocusLost,
                        sf::Event::FocusGained,
                        sf::Event::TextEntered,
                        sf::Event::KeyPressed,
                        sf::Event::KeyReleased,
                        sf::Event::MouseWheelScrolled,
                        sf::Event::MouseButtonPressed,
                        sf::Event::MouseButtonReleased,
                        sf::Event::MouseMoved,
                        sf::Event::MouseMovedRaw,
                        sf::Event::MouseEntered,
                        sf::Event::MouseLeft,
                        sf::Event::JoystickButtonPressed,
                        sf::Event::JoystickButtonReleased,
                        sf::Event::JoystickMoved,
                        sf::Event::JoystickConnected,
                        sf::Event::JoystickDisconnected,
                        sf::Event::TouchBegan,
                        sf::Event::TouchMoved,
                        sf::Event::TouchEnded,
                        sf::Event::SensorChanged>(*event);
                }

                publish_event(TickEvent{ m_frame_duration });

                while (!m_msg_queue.empty())
                {
                    Msg msg = m_msg_queue.front();
                    m_msg_queue.pop_front();
                    if (on_msg)
                    {
                        on_msg(m_window, msg);
                    }
                    handle_event(update(m_model_state, msg));
                }
            }

            m_window.clear();
            render(m_model_state, m_window);
            m_window.display();
        }
    }

    template <class Event>
    using event_handler_t = std::function<UpdateResult(const Model&, const Event&)>;

    using event_handler_ptr = std::function<UpdateResult(const Model&, const void*)>;

    std::map<std::type_index, event_handler_ptr> m_subscriptions;

    template <class Event>
    auto subscribe(event_handler_t<Event> event_handler)
    {
        m_subscriptions.emplace(  //
            std::type_index{ typeid(Event) },
            [=](const Model& m, const void* ptr) -> UpdateResult
            { return event_handler(m, *static_cast<const Event*>(ptr)); });
    }

    template <class Event>
    void publish_event(const Event& event)
    {
        const auto [b, e] = m_subscriptions.equal_range(std::type_index{ typeid(Event) });
        for (auto it = b; it != e; ++it)
        {
            handle_event(it->second(m_model_state, &event));
        }
    }
};
