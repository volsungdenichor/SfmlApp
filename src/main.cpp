#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>
#include <ferrugo/core/functional.hpp>
#include <ferrugo/core/sequence.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

#include "animation.hpp"
#include "app_runner.hpp"
#include "canvas_item.hpp"
// #include "event_handler.hpp"
#include "vec_t.hpp"

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
        vec_t location = {};
        vec_t velocity = {};
        vec_t accelarion = {};
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

struct Model
{
    std::vector<Boid> boids;
    float max_angular_velocity;
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
    vec_t pos;
};

}  // namespace Commands

using Command = std::variant<  //
    Commands::Init,
    Commands::Exit,
    Commands::Accelerate,
    Commands::AddBoid>;

template <class Model>
auto render_model(const sf::Font& font, const std::function<foo::CanvasItem(const Model&)>& func)
    -> std::function<void(const Model&, sf::RenderWindow&)>
{
    return [&](const Model& m, sf::RenderWindow& window)
    {
        auto ctx = foo::Context{ window };
        const auto state = foo::State{ foo::Style{}, foo::TextStyle{ font }, sf::RenderStates{} };
        const auto scene = func(m);
        scene(ctx, state);
    };
}

namespace detail
{

struct apply_fn
{
    template <class Func>
    struct impl_t
    {
        Func m_func;

        template <class T>
        auto operator()(T& item) const -> T&
        {
            std::invoke(m_func, item);
            return item;
        }

        template <class T>
        friend auto operator|(T& item, const impl_t& impl) -> T&
        {
            return impl(std::move(item));
        }
    };

    template <class Func>
    auto operator()(Func&& func) const -> impl_t<std::decay_t<Func>>
    {
        return { std::forward<Func>(func) };
    }
};
struct with_fn
{
    template <class Func>
    struct impl_t
    {
        Func m_func;

        template <class T>
        auto operator()(T item) const -> T
        {
            std::invoke(m_func, item);
            return item;
        }

        template <class T>
        friend auto operator|(T item, const impl_t& impl) -> T
        {
            return impl(std::move(item));
        }
    };

    template <class Func>
    auto operator()(Func&& func) const -> impl_t<std::decay_t<Func>>
    {
        return { std::forward<Func>(func) };
    }
};

}  // namespace detail

static constexpr auto apply = detail::apply_fn{};
static constexpr auto with = detail::with_fn{};

template <class T>
constexpr auto create(std::function<void(T&)> func) -> T
{
    return T{} | with(std::move(func));
}

auto get_center(sf::Vector2u desktop_size, sf::Vector2u window_size) -> sf::Vector2i
{
    return { (int)(desktop_size.x / 2 - window_size.x / 2), (int)(desktop_size.y / 2 - window_size.y / 2) };
}

constexpr auto model_to_canvas_item = [](const Model& m) -> foo::CanvasItem
{
    return foo::group(
        foo::text(str("n = ", std::fixed, m.boids.size()))  //
            | foo::translate({ 8, 8 + 16 * 0 })             //
            | foo::fill_color(sf::Color::Red)               //
            | foo::outline_color(sf::Color::Red)            //
            | foo::font_size(16),
        foo::transform(
            [](const Boid& b) -> foo::CanvasItem
            {
                const auto size = vec_t{ 20.F, 30.F };
                return foo::rect(size)                       //
                       | foo::fill_color(sf::Color::Yellow)  //
                       | foo::translate(-size / 2)           //
                       | foo::rotate(b.angular.location)     //
                       | foo::translate(b.linear.location);
            },
            m.boids));
};

constexpr auto update = [](Model m, const Command& cmd) -> std::tuple<Model, std::optional<Command>>
{
    if (const auto c = std::get_if<Commands::Exit>(&cmd))
    {
        std::cout << "Bye!"
                  << "\n";
        return { m, {} };
    }
    if (const auto c = std::get_if<Commands::Accelerate>(&cmd))
    {
        for (auto& b : m.boids)
        {
            b.angular.acceleration = c->value;
        }
        return { m, {} };
    }
    if (const auto c = std::get_if<Commands::AddBoid>(&cmd))
    {
        m.boids.push_back(create<Boid>(
            [&](Boid& b)
            {
                b.linear.location = c->pos;
                b.angular.acceleration = 0.1F;
            }));
        return { m, {} };
    }
    return { m, {} };
};

constexpr auto on_tick = [](Model m, const TickEvent& event) -> std::tuple<Model, std::optional<Command>>
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
    return { std::move(m), {} };
};

constexpr auto on_key_pressed = [](Model m, const sf::Event::KeyPressed& e) -> std::tuple<Model, std::optional<Command>>
{
    static const auto commands = std::map<sf::Keyboard::Key, Command>{
        { sf::Keyboard::Key::Escape, Commands::Exit{} },
        { sf::Keyboard::Key::Q, Commands::Exit{} },
        { sf::Keyboard::Key::Up, Commands::Accelerate{ +1.F } },
        { sf::Keyboard::Key::Down, Commands::Accelerate{ -1.F } },
    };
    if (const auto commmand = commands.find(e.code); commmand != commands.end())
    {
        return { std::move(m), commmand->second };
    }
    return { std::move(m), {} };
};

constexpr auto on_mouse_buttton_pressed
    = [](Model m, const sf::Event::MouseButtonPressed& e) -> std::tuple<Model, std::optional<Command>>
{
    if (e.button == sf::Mouse::Button::Left)
    {
        return { std::move(m), Commands::AddBoid{ convert(e.position) } };
    }
    return { std::move(m), {} };
};

void run(const std::vector<std::string_view> args)
{
    static const std::string fonts_dir = R"(/mnt/c/Windows/Fonts/)";

    auto window = sf::RenderWindow(sf::VideoMode({ 1024, 768 }), "CMake SFML Project");
    const auto desktop_size = sf::VideoMode::getDesktopMode().size;
    window.setPosition(get_center(sf::VideoMode::getDesktopMode().size, window.getSize()));

    const sf::Font arial = load_font(fonts_dir + "arial.ttf");

    using namespace ferrugo;

    const auto frame_duration = duration_t{ 0.01F };

    auto boids = std::vector<Boid>{ create<Boid>(
                                        [](Boid& b)
                                        {
                                            b.angular.velocity = 1.0F;
                                            b.linear.location = { 300, 300 };
                                        }),
                                    create<Boid>(
                                        [](Boid& b)
                                        {
                                            b.angular.velocity = -1.0F;
                                            b.linear.location = { 500, 100 };
                                            b.linear.accelarion = { -1.F, 0.5F };
                                        }),
                                    create<Boid>(
                                        [](Boid& b)
                                        {
                                            b.angular.velocity = 2.0F;
                                            b.linear.location = { 400, 200 };
                                        }) };

    auto model = Model{ .boids = std::move(boids), .max_angular_velocity = 1.5F };

    auto app = App<Model, Command>{ window, std::move(model) };
    app.render = render_model<Model>(arial, model_to_canvas_item);

    app.on_msg = [](sf::RenderWindow& w, const Command& cmd)
    {
        if (const auto c = std::get_if<Commands::Exit>(&cmd))
        {
            w.close();
        }
    };

    app.update = update;

    app.subscribe<InitEvent>(
        [](Model m, const InitEvent&) -> std::tuple<Model, std::optional<Command>> {
            return { std::move(m), Commands::Init{} };
        });

    app.subscribe<TickEvent>(on_tick);
    app.subscribe<sf::Event::KeyPressed>(on_key_pressed);
    app.subscribe<sf::Event::MouseButtonPressed>(on_mouse_buttton_pressed);

    app.run();
}

int main(int argc, char* argv[])
{
    try
    {
        std::vector<std::string_view> args;
        for (int n = 0; n < argc; ++n)
        {
            args.push_back(argv[n]);
        }
        run(args);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}
