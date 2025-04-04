#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>
#include <functional>

using event_handler_fn = std::function<void(sf::RenderWindow&, const sf::Event&)>;
using render_fn = std::function<void(sf::RenderWindow&, float)>;
using update_fn = std::function<void(float)>;

inline void run_app(
    sf::RenderWindow& window,
    const event_handler_fn& event_handler,
    const update_fn& updater,
    const render_fn& renderer,
    float time_per_frame)
{
    sf::Clock clock;
    auto time_since_last_update = 0.F;

    sf::Event event{};

    while (window.isOpen())
    {
        const auto elapsed = clock.restart().asSeconds();
        time_since_last_update += elapsed;

        const auto fps = 1.0F / elapsed;

        while (time_since_last_update > time_per_frame)
        {
            time_since_last_update -= time_per_frame;

            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }

                event_handler(window, event);
            }

            updater(time_per_frame);
        }

        window.clear();
        renderer(window, fps);
        window.display();
    }
}
