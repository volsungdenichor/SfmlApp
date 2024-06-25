#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <ferrugo/core/format.hpp>
#include <ferrugo/core/pipeline.hpp>
#include <functional>

namespace ferrugo::core
{
template <class T>
struct formatter<sf::Vector2<T>>
{
    void parse(const parse_context& ctx)
    {
    }

    void format(const format_context& ctx, const sf::Vector2<T>& item)
    {
    }
};

}  // namespace ferrugo::core

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

struct boid_t
{
    sf::Vector2f location;
    sf::Vector2f velocity;
    sf::Color color;
};

template <class T>
T create(const action_t<T&>& init)
{
    T result{};
    init(result);
    return result;
}

int main()
{
    std::vector<boid_t> boids;
    boids.push_back(boid_t{ sf::Vector2f{ 0, 0 }, sf::Vector2f{ 50, 50 }, sf::Color::Red });
    boids.push_back(boid_t{ sf::Vector2f{ 50, 1000 }, sf::Vector2f{ 40, -2 }, sf::Color::Green });
    auto window = sf::RenderWindow{ { 1920u, 1080u }, "CMake SFML Project" };

    ferrugo::core::print("{}")(window.getSize());
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
            const float scale = 50.F;
            for (auto& b : boids)
            {
                b.location.x += b.velocity.x * dt * scale;
                b.location.y += b.velocity.y * dt * scale;

                if (b.location.x > window.getView().getSize().x || b.location.x < 0)
                {
                    b.velocity.x *= -1;
                }
                if (b.location.y > window.getView().getSize().y || b.location.y < 0)
                {
                    b.velocity.y *= -1;
                }
            }
        },
        [&](sf::RenderWindow& w)
        {
            for (const auto& b : boids)
            {
                w.draw(create<sf::CircleShape>(
                    [&](auto& it)
                    {
                        it.setRadius(10.F);
                        it.setPosition(b.location);
                        it.setFillColor(b.color);
                    }));
            }
        },
        0.01F);
}
