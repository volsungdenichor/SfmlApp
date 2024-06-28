#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>

#include "animation.hpp"
#include "app_runner.hpp"
#include "event_handler_t.hpp"
#include "widget_builders.hpp"

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
    const auto anim = repeat(
        sequence(
            gradual(0.F, 200.F, duration_t{ 10.F }, ease::cubic_out),
            constant(200.F, duration_t{ 10.F }),
            gradual(200.F, 0.F, duration_t{ 5.F }, ease::cubic_in)),
        3.F);

    float angle = 0.F;
    float anim_time = 0.F;
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
    event_handler_t event_handler{};
    event_handler.on_close = [](sf::RenderWindow& w) { w.close(); };
    event_handler.on_key_pressed = [&](sf::RenderWindow& w, const sf::Event::KeyEvent& e)
    {
        if (e.code == sf::Keyboard::Escape)
        {
            w.close();
        }

        if (e.code == sf::Keyboard::Num1)
        {
            boids[0].velocity = -boids[0].velocity;
        }

        if (e.code == sf::Keyboard::Num2)
        {
            boids[1].velocity = -boids[1].velocity;
        }
    };

    run_app(
        window,
        event_handler,
        [&](float dt)
        {
            anim_time += dt * 10.F;
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

            drawables.push_back(circle(200.0) | texture({ txt, sf::IntRect{ 40, 10, 200, 200 } }) | position({ 500, 100 }));
            drawables.push_back(sprite({ txt, sf::IntRect{ 40, 10, 200, 200 } }));
            drawables.push_back(
                text("Hello World!", arial, 24) | outline(sf::Color::Red) | fill(sf::Color::White) | outline_thickness(1.F)
                | position({ 1200, 400 }));

            drawables.push_back(
                text("fps = " + std::to_string(fps), verdana, 12) | bold() | italic() | fill(sf::Color::White)
                | position({ 1200, 100 }));

            drawables.push_back(circle(10) | fill(sf::Color::White) | position({ anim(anim_time), 500 }));

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