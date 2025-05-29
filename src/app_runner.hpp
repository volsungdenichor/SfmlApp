#pragma once

#include <SFML/Graphics.hpp>
#include <deque>
#include <functional>
#include <optional>

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
    using Event = std::variant<sf::Event, TickEvent, InitEvent>;
    using UpdateResult = std::tuple<Model, std::optional<Msg>>;
    using RenderFn = std::function<void(const Model&, sf::RenderWindow&)>;
    using HandleEventFn = std::function<UpdateResult(const Model&, const Event&)>;
    using UpdateFn = std::function<UpdateResult(const Model&, const Msg&)>;
    using HandleMsgFn = std::function<void(sf::RenderWindow&, const Msg&)>;

    sf::RenderWindow& m_window;
    Model m_model_state;
    RenderFn render = {};
    HandleEventFn m_handle_event;
    UpdateFn m_update;
    HandleMsgFn m_handle_msg;
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

    void run()
    {
        sf::Clock clock;
        duration_t time_since_last_update = 0.F;

        handle_event(m_handle_event(m_model_state, InitEvent{}));

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
                    handle_event(m_handle_event(m_model_state, *event));
                }

                handle_event(m_handle_event(m_model_state, TickEvent{ m_frame_duration }));

                while (!m_msg_queue.empty())
                {
                    Msg msg = m_msg_queue.front();
                    m_msg_queue.pop_front();
                    if (m_handle_msg)
                    {
                        m_handle_msg(m_window, msg);
                    }
                    handle_event(m_update(m_model_state, msg));
                }
            }

            m_window.clear();
            render(m_model_state, m_window);
            m_window.display();
        }
    }
};
