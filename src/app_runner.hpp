#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>

#include "defs.hpp"

inline void run_app(
    sf::RenderWindow& window,
    const action_t<sf::RenderWindow&, const sf::Event&>& event_handler,
    const action_t<float>& updater,
    const action_t<sf::RenderWindow&, float>& renderer,
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