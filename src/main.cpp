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

struct Model
{
    int angular_acceleration = 0;
    float angular_velocity = 0;
    float angle = 0.F;
    float max_angular_velocity;
};

enum class Command
{
    init,
    exit,
    up,
    down
};

struct ModelEventHandler
{
    auto operator()(Model m, const sf::Event::KeyPressed& e) const -> std::tuple<Model, std::optional<Command>>
    {
        if (e.code == sf::Keyboard::Key::Escape)
        {
            return { std::move(m), Command::exit };
        }
        if (e.code == sf::Keyboard::Key::Up)
        {
            return { std::move(m), Command::up };
        }
        if (e.code == sf::Keyboard::Key::Down)
        {
            return { m, Command::down };
        }
        return { std::move(m), {} };
    }

    auto operator()(Model m, const sf::Event::KeyReleased& e) const -> std::tuple<Model, std::optional<Command>>
    {
        return { std::move(m), {} };
    }

    auto operator()(Model m, const sf::Event& event) const -> std::tuple<Model, std::optional<Command>>
    {
        if (const auto e = event.getIf<sf::Event::KeyPressed>())
        {
            return (*this)(std::move(m), *e);
        }
        if (const auto e = event.getIf<sf::Event::KeyReleased>())
        {
            return (*this)(std::move(m), *e);
        }
        return { std::move(m), {} };
    };

    auto operator()(Model m, const TickEvent& event) const -> std::tuple<Model, std::optional<Command>>
    {
        m.angular_velocity += m.angular_acceleration * event.elapsed;
        m.angular_velocity = std::min(m.angular_velocity, +m.max_angular_velocity);
        m.angular_velocity = std::max(m.angular_velocity, -m.max_angular_velocity);
        m.angle += m.angular_velocity * event.elapsed;
        return { std::move(m), {} };
    }

    auto operator()(Model m, const InitEvent& event) const -> std::tuple<Model, std::optional<Command>>
    {
        return { std::move(m), Command::init };
    }

    auto operator()(Model m, const std::variant<sf::Event, TickEvent, InitEvent>& event) const
        -> std::tuple<Model, std::optional<Command>>
    {
        if (const auto e = std::get_if<sf::Event>(&event))
        {
            return (*this)(std::move(m), *e);
        }
        if (const auto e = std::get_if<TickEvent>(&event))
        {
            return (*this)(std::move(m), *e);
        }
        if (const auto e = std::get_if<InitEvent>(&event))
        {
            return (*this)(std::move(m), *e);
        }
        return { std::move(m), {} };
    };
};

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

void run()
{
    auto window = sf::RenderWindow(sf::VideoMode({ 1024, 768 }), "CMake SFML Project");
    const auto desktop = sf::VideoMode::getDesktopMode();
    window.setPosition(
        { (int)(desktop.size.x / 2 - window.getSize().x / 2), (int)(desktop.size.y / 2 - window.getSize().y / 2) });

    const sf::Font arial = load_font(R"(/mnt/c/Windows/Fonts/arial.ttf)");
    const sf::Font verdana = load_font(R"(/mnt/c/Windows/Fonts/verdana.ttf)");

    using namespace ferrugo;

    const auto frame_duration = duration_t{ 0.01F };

    auto app = App<Model, Command>{ window, Model{ .max_angular_velocity = 1.5F } };
    app.render = render_model<Model>(
        arial,
        [](const Model& m) -> foo::CanvasItem
        {
            return foo::group(
                foo::group(
                    foo::text(str("a = ", std::fixed, m.angular_acceleration)) | foo::translate({ 8, 8 + 16 * 0 }),
                    foo::text(str("v = ", std::fixed, m.angular_velocity)) | foo::translate({ 8, 8 + 16 * 1 }))
                    | foo::fill_color(sf::Color::Yellow),

                foo::group(
                    foo::circle(10.F) | foo::fill_color(sf::Color::Yellow) | foo::translate({ 200, 100 }),
                    foo::circle(10.F) | foo::fill_color(sf::Color::Red) | foo::translate({ 300, 100 }),
                    foo::circle(10.F) | foo::fill_color(sf::Color::Blue) | foo::translate({ 200, 400 }),
                    foo::circle(10.F) | foo::fill_color(sf::Color::Green) | foo::translate({ 300, 400 }))
                    | foo::rotate(m.angle, { 512, 384 }));
        });

    app.m_handle_msg = [](sf::RenderWindow& w, const Command& cmd)
    {
        if (cmd == Command::exit)
        {
            std::cout << "I want to exit!"
                      << "\n";
            w.close();
        }
    };

    app.m_update = [&](Model m, const Command& cmd) -> std::tuple<Model, std::optional<Command>>
    {
        if (cmd == Command::init)
        {
            std::cout << "Hello"
                      << "\n";
            return { m, {} };
        }
        if (cmd == Command::up)
        {
            m.angular_acceleration = +1;
            return { m, {} };
        }
        if (cmd == Command::down)
        {
            m.angular_acceleration = -1;
            return { m, {} };
        }
        return { m, {} };
    };

    // auto event_handler = EventHandler<Model, Command>{};
    app.m_handle_event = ModelEventHandler{};

    app.run();
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