#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <functional>
#include <iostream>
#include <memory>

#include "widget_builders.hpp"

void run_app(
    sf::RenderWindow& window,
    const action_t<const sf::Event&>& event_handler,
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

                event_handler(event);
            }

            updater(time_per_frame);
        }

        window.clear();
        renderer(window, fps);
        window.display();
    }
}

struct boid_t
{
    vec_t location = {};
    vec_t velocity = {};
    sf::Color color = sf::Color::White;
};

template <class T>
T create(const action_t<T&>& init)
{
    T result{};
    init(result);
    return result;
}

sf::Texture load_texture(const std::string& path)
{
    sf::Texture result{};
    if (!result.loadFromFile(path))
    {
        throw std::runtime_error{ "Unable to load texture from " + path };
    }
    return result;
}

sf::Font load_font(const std::string& path)
{
    sf::Font result{};
    if (!result.loadFromFile(path))
    {
        throw std::runtime_error{ "Unable to load font from " + path };
    }
    return result;
}

void run()
{
    float angle = 0.F;
    std::vector<boid_t> boids = { create<boid_t>(
                                      [](auto& it)
                                      {
                                          it.location = vec_t(0, 0);
                                          it.velocity = vec_t(50, 75);
                                          it.color = sf::Color::Red;
                                      }),
                                  create<boid_t>(
                                      [](auto& it)
                                      {
                                          it.location = vec_t(50, 1000);
                                          it.velocity = vec_t(40, -10);
                                          it.color = sf::Color::Green;
                                      }),
                                  create<boid_t>(
                                      [](auto& it)
                                      {
                                          it.location = vec_t(500, 100);
                                          it.velocity = vec_t(-40, 20);
                                          it.color = sf::Color::Yellow;
                                      })

    };

    auto window = sf::RenderWindow{ { 1920u, 1080u }, "CMake SFML Project" };

    static const auto render_boid
        = [](const boid_t& b) -> widget_t { return circle(10.F) | position(b.location) | fill(b.color); };

    const sf::Texture txt = load_texture(R"(/mnt/d/Users/Krzysiek/Pictures/conan.bmp)");
    const sf::Font arial = load_font(R"(/mnt/c/Windows/Fonts/arial.ttf)");
    const sf::Font verdana = load_font(R"(/mnt/c/Windows/Fonts/verdana.ttf)");

    run_app(
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
            angle += dt * 50.F;
            static const float scale = 20.F;
            for (auto& b : boids)
            {
                b.location += b.velocity * dt * scale;

                if (b.location.x() > window.getView().getSize().x || b.location.x() < 0)
                {
                    b.velocity = vec_t(-b.velocity.x(), b.velocity.y());
                }
                if (b.location.y() > window.getView().getSize().y || b.location.y() < 0)
                {
                    b.velocity = vec_t(b.velocity.x(), -b.velocity.y());
                }
            }
        },
        [&](sf::RenderWindow& w, float fps)
        {
            std::vector<drawable_t> drawables;
            drawables.push_back(circle(200) | fill(sf::Color::Yellow) | outline(sf::Color::Red) | outline_thickness(5.0));
            drawables.push_back(
                rect(50.F, 20.F) | fill(sf::Color::Red) | outline(sf::Color::Yellow) | position({ 100, 50 }) | rotate(45.F));
            drawables.push_back(circle(50.F) | fill(sf::Color::Green) | position({ 200, 50 }));
            drawables.push_back(
                polygon({ vec_t(0, 0),   vec_t(20, 0),  vec_t(30, 10), vec_t(50, 10), vec_t(60, 0),
                          vec_t(80, 0),  vec_t(80, 20), vec_t(70, 30), vec_t(70, 50), vec_t(80, 60),
                          vec_t(80, 80), vec_t(60, 80), vec_t(50, 70), vec_t(30, 70), vec_t(20, 80),
                          vec_t(0, 80),  vec_t(0, 60),  vec_t(10, 50), vec_t(10, 30), vec_t(0, 20) })
                | position({ 200, 200 }) | rotate(angle) | fill(sf::Color::Red));
            for (const auto& b : boids)
            {
                drawables.push_back(render_boid(b));
            }

            drawables.push_back(circle(200.0) | texture(&txt, sf::IntRect{ 40, 10, 200, 200 }) | position({ 500, 100 }));
            drawables.push_back(sprite(txt, sf::IntRect{ 40, 10, 200, 200 }));
            drawables.push_back(
                text("Hello World!", arial, 24) | outline(sf::Color::White) | fill(sf::Color::Red) | outline_thickness(1.F)
                | position({ 1200, 400 }));

            drawables.push_back(
                text("fps = " + std::to_string(fps), verdana, 12) | fill(sf::Color::White) | position({ 1200, 100 }));

            for (const auto& d : drawables)
            {
                d(w);
            }
        },
        0.01F);
}

int main()
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}