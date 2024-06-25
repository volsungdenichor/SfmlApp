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

    void format(format_context& ctx, const sf::Vector2<T>& item)
    {
        write_to(ctx, "[", item.x, ", ", item.y, "]");
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

struct vec : std::array<float, 2>
{
    using base_t = std::array<float, 2>;
    vec(float x, float y) : base_t{ { x, y } }
    {
    }

    vec() : vec(0.F, 0.F)
    {
    }

    float x() const
    {
        return (*this)[0];
    }

    float y() const
    {
        return (*this)[1];
    }

    operator sf::Vector2f() const
    {
        return sf::Vector2f(x(), y());
    }

    friend vec operator+(const vec& item)
    {
        return item;
    }

    friend vec operator-(vec item)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            item[i] = -item[i];
        }
        return item;
    }

    friend vec& operator*=(vec& lhs, float rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] *= rhs;
        }
        return lhs;
    }

    friend vec operator*(vec lhs, float rhs)
    {
        return lhs *= rhs;
    }

    friend vec& operator/=(vec& lhs, float rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] /= rhs;
        }
        return lhs;
    }

    friend vec operator/(vec lhs, float rhs)
    {
        return lhs /= rhs;
    }

    friend vec operator*(float lhs, const vec& rhs)
    {
        return rhs * lhs;
    }

    friend vec& operator+=(vec& lhs, const vec& rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] += rhs[i];
        }
        return lhs;
    }

    friend vec operator+(vec lhs, const vec& rhs)
    {
        return lhs += rhs;
    }

    friend vec& operator-=(vec& lhs, const vec& rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] -= rhs[i];
        }
        return lhs;
    }

    friend vec operator-(vec lhs, const vec& rhs)
    {
        return lhs -= rhs;
    }
};

struct boid_t
{
    vec location = {};
    vec velocity = {};
    sf::Color color = sf::Color::White;
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
    std::vector<boid_t> boids = { create<boid_t>(
                                      [](auto& it)
                                      {
                                          it.location = vec(0, 0);
                                          it.velocity = vec(50, 75);
                                          it.color = sf::Color::Red;
                                      }),
                                  create<boid_t>(
                                      [](auto& it)
                                      {
                                          it.location = vec(50, 1000);
                                          it.velocity = vec(40, -10);
                                          it.color = sf::Color::Green;
                                      }),
                                  create<boid_t>(
                                      [](auto& it)
                                      {
                                          it.location = vec(500, 100);
                                          it.velocity = vec(-40, 20);
                                          it.color = sf::Color::Yellow;
                                      })

    };

    auto window = sf::RenderWindow{ { 1920u, 1080u }, "CMake SFML Project" };

    ferrugo::core::println("size={}")(window.getSize());

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
            static const float scale = 20.F;
            for (auto& b : boids)
            {
                b.location += b.velocity * dt * scale;

                if (b.location.x() > window.getView().getSize().x || b.location.x() < 0)
                {
                    b.velocity = vec(-b.velocity.x(), b.velocity.y());
                }
                if (b.location.y() > window.getView().getSize().y || b.location.y() < 0)
                {
                    b.velocity = vec(b.velocity.x(), -b.velocity.y());
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
