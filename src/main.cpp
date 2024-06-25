#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <functional>

template <class... Args>
using action_t = std::function<void(Args...)>;

void run(
    sf::RenderWindow& window,
    const action_t<const sf::Event&>& event_handler,
    const action_t<float>& updater,
    const action_t<sf::RenderWindow&>& renderer,
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

                event_handler(event);
            }

            updater(time_per_frame);
        }

        window.clear();
        renderer(window);
        window.display();
    }
}

using vector_t = std::array<float, 2>;

struct boid_t
{
    vector_t location;
    vector_t velocity;
};

int main()
{
    std::vector<boid_t> boids;
    boids.push_back(boid_t{ { 0, 0 }, { 50, 1 } });
    boids.push_back(boid_t{ { 50, 1000 }, { 40, -2 } });
    auto window = sf::RenderWindow{ { 1920u, 1080u }, "CMake SFML Project" };
    run(
        window,
        [&](const sf::Event& event)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
            }
        },
        [&](float dt)
        {
            const float scale = 100.F;
            for (auto& b : boids)
            {
                for (std::size_t i = 0; i < 2; ++i)
                {
                    b.location[i] += b.velocity[i] * dt * scale;
                }
                if (b.location[0] > window.getView().getSize().x ||  b.location[0] < 0)
                {
                    b.velocity[0] *= -1;
                }
                if (b.location[1] > window.getView().getSize().y ||  b.location[1] < 0)
                {
                    b.velocity[1] *= -1;
                }
            }
        },
        [&](sf::RenderWindow& w)
        {
            for (const auto& b : boids)
            {
                sf::CircleShape shape(10.f);
                shape.setPosition(sf::Vector2f{ b.location[0], b.location[1] });
                shape.setFillColor(sf::Color::Green);
                w.draw(shape);
            }
        },
        0.01F);
}
