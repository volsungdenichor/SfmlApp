#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>
#include <functional>

using fps_t = float;       // 1/s
using duration_t = float;  // s

using event_handler_fn = std::function<void(sf::RenderWindow&, const sf::Event&)>;
using render_fn = std::function<void(sf::RenderWindow&, fps_t)>;
using update_fn = std::function<void(duration_t)>;

inline void run_app(
    sf::RenderWindow& window,
    const event_handler_fn& event_handler,
    const update_fn& updater,
    const render_fn& renderer,
    duration_t frame_duration)
{
    sf::Clock clock;
    duration_t time_since_last_update = 0.F;

    sf::Event event{};

    while (window.isOpen())
    {
        const duration_t elapsed = clock.restart().asSeconds();
        time_since_last_update += elapsed;

        const fps_t fps = 1.0F / elapsed;

        while (time_since_last_update > frame_duration)
        {
            time_since_last_update -= frame_duration;

            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }

                event_handler(window, event);
            }

            updater(frame_duration);
        }

        window.clear();
        renderer(window, fps);
        window.display();
    }
}
