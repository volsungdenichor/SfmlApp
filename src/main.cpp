#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <variant>

#include "animation.hpp"
#include "app_runner.hpp"
#include "canvas_item.hpp"
#include "functional.hpp"

template <class... Args>
auto str(Args&&... args) -> std::string
{
    std::stringstream ss;
    (ss << ... << std::forward<Args>(args));
    return ss.str();
}

inline auto load_texture(const std::string& path) -> sf::Texture
{
    sf::Texture result{};
    if (!result.loadFromFile(path))
    {
        throw std::runtime_error{ "Unable to load texture from " + path };
    }
    return result;
}

inline auto load_font(const std::string& path) -> sf::Font
{
    sf::Font result{};
    if (!result.openFromFile(path))
    {
        throw std::runtime_error{ "Unable to load font from " + path };
    }
    return result;
}

struct Boid
{
    struct Linear
    {
        mx::vector_2d<float> location = {};
        mx::vector_2d<float> velocity = {};
        mx::vector_2d<float> accelarion = {};
    };
    struct Angular
    {
        float location = 0.F;
        float velocity = 0;
        float acceleration = 0;
    };

    Linear linear;
    Angular angular;
};

struct Point
{
    mx::vector_2d<float> pos;
    float y;
    anim::animation<float> animation = anim::constant(0.F, 100.F);
};

struct Model
{
    anim::time_point_t time_point;
    std::vector<Boid> boids;
    std::vector<Point> points;
    float max_angular_velocity;
    bool show_info = false;
};

namespace Commands
{

struct Init
{
};
struct Exit
{
};
struct Accelerate
{
    float value;
};

struct AddBoid
{
    mx::vector_2d<float> pos;
};

struct ToggleInfo
{
};

}  // namespace Commands

using Command = std::variant<  //
    Commands::Init,
    Commands::Exit,
    Commands::Accelerate,
    Commands::AddBoid,
    Commands::ToggleInfo>;

template <class Model>
auto render_model(const sf::Font& font, const std::function<canvas::CanvasItem(const Model&, fps_t)>& func)
    -> RendererFn<Model>
{
    return [=](sf::RenderWindow& window, const Model& m, fps_t fps)
    {
        auto ctx = canvas::Context{ window };
        const auto state = canvas::State{ canvas::Style{}, canvas::TextStyle{ font }, sf::RenderStates{} };
        const auto scene = func(m, fps);
        scene(ctx, state);
    };
}

auto get_center(sf::Vector2u desktop_size, sf::Vector2u window_size) -> sf::Vector2i
{
    return { (int)(desktop_size.x / 2 - window_size.x / 2), (int)(desktop_size.y / 2 - window_size.y / 2) };
}

auto create_model() -> Model
{
    Model m{};
    m.max_angular_velocity = 1.5F;
    m.time_point = anim::time_point_t{ 0.F };
    for (int y = 100; y < 700; y += 15)
    {
        for (int x = 100; x < 900; x += 15)
        {
            m.boids.push_back(create<Boid>(
                [&](Boid& b)
                {
                    b.angular.velocity = ((x / 20) % 2 == 0) ? 1.0F : -1.0F;
                    b.linear.location = mx::vector_2d<float>{ (float)x, (float)y };
                }));
        }
    }
    m.points = {
        create<Point>(
            [](Point& p)
            {
                p.animation = anim::ping_pong(anim::gradual(0.F, 500.F, anim::duration_t{ 1 }, anim::ease::none), 10.0F);
                p.y = 50;
            }),
        create<Point>(
            [](Point& p)
            {
                p.animation
                    = anim::ping_pong(anim::gradual(0.F, 500.F, anim::duration_t{ 1 }, anim::ease::quad_in_out), 10.0F);
                p.y = 100;
            }),
        create<Point>(
            [](Point& p)
            {
                p.animation = anim::ping_pong(anim::gradual(0.F, 500.F, anim::duration_t{ 1 }, anim::ease::quad_in), 10.0F);
                p.y = 150;
            }),
        create<Point>(
            [](Point& p)
            {
                p.animation = anim::ping_pong(anim::gradual(0.F, 500.F, anim::duration_t{ 1 }, anim::ease::quad_out), 10.0F);
                p.y = 200;
            }),
        create<Point>(
            [](Point& p)
            {
                p.animation
                    = anim::ping_pong(anim::gradual(0.F, 500.F, anim::duration_t{ 1 }, anim::ease::cubic_in_out), 10.0F);
                p.y = 300;
            }),
        create<Point>(
            [](Point& p)
            {
                p.animation = anim::ping_pong(anim::gradual(0.F, 500.F, anim::duration_t{ 1 }, anim::ease::cubic_in), 10.0F);
                p.y = 350;
            }),
        create<Point>(
            [](Point& p)
            {
                p.animation
                    = anim::ping_pong(anim::gradual(0.F, 500.F, anim::duration_t{ 1 }, anim::ease::cubic_out), 10.0F);
                p.y = 400;
            }),
        create<Point>(
            [](Point& p)
            {
                p.animation
                    = anim::ping_pong(anim::gradual(0.F, 500.F, anim::duration_t{ 1 }, anim::ease::circ_in_out), 10.0F);
                p.y = 500;
            })

    };
    return m;
}

void run(const std::vector<std::string_view> args)
{
    static const std::string fonts_dir = R"(/mnt/c/Windows/Fonts/)";

    auto window = sf::RenderWindow(sf::VideoMode({ 1024, 768 }), "CMake SFML Project");
    const auto desktop_size = sf::VideoMode::getDesktopMode().size;
    window.setPosition(get_center(sf::VideoMode::getDesktopMode().size, window.getSize()));

    const sf::Font font = load_font(fonts_dir + "arial.ttf");

    auto app = App<Model, Command>{ window, create_model() };
    app.frame_duration = duration_t{ 0.01F };

    app.render = render_model<Model>(
        font,
        [](const Model& m, fps_t fps) -> canvas::CanvasItem
        {
            const auto info = canvas::text(
                                  str("n = ",
                                      std::fixed,
                                      m.boids.size(),
                                      "\nt = ",
                                      std::fixed,
                                      m.time_point,
                                      "s\nfps = ",
                                      std::fixed,
                                      fps))                                    //
                              | canvas::translate({ 8, 8 })                    //
                              | canvas::fill_color(sf::Color::Red)             //
                              | canvas::outline_color(sf::Color::Transparent)  //
                              | canvas::outline_thickness(1.F)                 //
                              | canvas::font_size(12);
            return canvas::group(
                m.show_info ? info : canvas::empty_item(),
                canvas::transform(
                    [](const Boid& b) -> canvas::CanvasItem
                    {
                        const auto size = mx::vector_2d<float>{ 5.F, 7.F };
                        return canvas::rect(size)                       //
                               | canvas::fill_color(sf::Color::Yellow)  //
                               | canvas::translate(-size / 2)           //
                               | canvas::rotate(b.angular.location)     //
                               | canvas::translate(b.linear.location);
                    },
                    m.boids),
                canvas::transform(
                    [](const Point& p) -> canvas::CanvasItem
                    { return canvas::circle(15.F) | canvas::translate(p.pos) | canvas::fill_color(sf::Color::Green); },
                    m.points));
        });
    app.on_msg = [](sf::RenderWindow& w, const Command& cmd)
    {
        if (const auto c = std::get_if<Commands::Exit>(&cmd))
        {
            w.close();
        }
    };

    app.update = [](Model& m, const Command& cmd) -> std::optional<Command>
    {
        if (const auto c = std::get_if<Commands::Exit>(&cmd))
        {
            std::cout << "Bye!"
                      << "\n";
            return {};
        }
        else if (const auto c = std::get_if<Commands::Accelerate>(&cmd))
        {
            for (auto& b : m.boids)
            {
                b.angular.acceleration = c->value;
            }
            return {};
        }
        else if (const auto c = std::get_if<Commands::AddBoid>(&cmd))
        {
            m.boids.push_back(create<Boid>(
                [&](Boid& b)
                {
                    b.linear.location = c->pos;
                    b.angular.acceleration = 0.25F;
                }));
            return {};
        }
        else if (const auto c = std::get_if<Commands::Init>(&cmd))
        {
            return {};
        }
        else if (const auto c = std::get_if<Commands::ToggleInfo>(&cmd))
        {
            m.show_info = !m.show_info;
            return {};
        }
        return {};
    };

    app.subscribe<InitEvent>([](Model& m, const InitEvent&) -> std::optional<Command> { return Commands::Init{}; });

    app.subscribe<TickEvent>(
        [](Model& m, const TickEvent& event) -> std::optional<Command>
        {
            for (auto& b : m.boids)
            {
                b.linear.velocity += b.linear.accelarion * event.elapsed;
                b.linear.location += b.linear.velocity * event.elapsed;

                b.angular.velocity += b.angular.acceleration * event.elapsed;
                b.angular.velocity = std::min(b.angular.velocity, +m.max_angular_velocity);
                b.angular.velocity = std::max(b.angular.velocity, -m.max_angular_velocity);
                b.angular.location += b.angular.velocity * event.elapsed;
            }
            m.time_point += event.elapsed;
            for (auto& p : m.points)
            {
                p.pos = mx::vector_2d<float>{ p.animation(m.time_point), p.y };
            }
            return {};
        });
    app.subscribe<sf::Event::KeyPressed>(
        [](Model& m, const sf::Event::KeyPressed& e) -> std::optional<Command>
        {
            static const auto commands = std::map<sf::Keyboard::Key, Command>{
                { sf::Keyboard::Key::Escape, Commands::Exit{} },
                { sf::Keyboard::Key::Q, Commands::Exit{} },
                { sf::Keyboard::Key::Up, Commands::Accelerate{ +1.F } },
                { sf::Keyboard::Key::Down, Commands::Accelerate{ -1.F } },
                { sf::Keyboard::Key::F, Commands::ToggleInfo{} },
            };
            if (const auto commmand = commands.find(e.code); commmand != commands.end())
            {
                return commmand->second;
            }
            return {};
        });
    app.subscribe<sf::Event::MouseButtonPressed>(
        [](Model& m, const sf::Event::MouseButtonPressed& e) -> std::optional<Command>
        {
            if (e.button == sf::Mouse::Button::Left)
            {
                return Commands::AddBoid{ convert_as<float>(e.position) };
            }
            return {};
        });

    app.run();
}

int main(int argc, char* argv[])
{
    try
    {
        run(std::vector<std::string_view>(argv, argv + argc));
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}
