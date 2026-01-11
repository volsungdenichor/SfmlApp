#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mx/dcel.hpp>
#include <sstream>
#include <variant>
#include <zx/functional.hpp>
#include <zx/sequence.hpp>

#include "animation.hpp"
#include "app_runner.hpp"
#include "canvas_item.hpp"

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
    std::vector<mx::vector<float, 2>> points;
    std::optional<mx::dcel<float>> dcel;
    std::optional<mx::dcel<float>> voronoi;

    void update()
    {
        try
        {
            dcel = mx::triangulate(points);
        }
        catch (const std::exception& e)
        {
            std::cout << "error on triangulation: " << e.what() << '\n';
            dcel = std::nullopt;
        }
        if (dcel)
        {
            try
            {
                voronoi = mx::voronoi(*dcel);
            }
            catch (const std::exception& e)
            {
                std::cout << "error on voronoi: " << e.what() << '\n';
                voronoi = std::nullopt;
            }
        }
    }
};

namespace Commands
{

struct Init
{
};
struct Exit
{
};
struct AddPoint
{
    mx::vector_2d<float> pos;
};

}  // namespace Commands

using Command = std::variant<  //
    Commands::Init,
    Commands::Exit,
    Commands::AddPoint>;

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
    m.points = {};
    m.dcel = std::nullopt;
    m.voronoi = std::nullopt;
    m.update();
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
            std::vector<canvas::CanvasItem> items;
            if (m.voronoi)
            {
                m.voronoi->faces()(
                    [&](const auto& face)
                    {
                        items.push_back(
                            canvas::polygon(face.as_polygon())            //
                            | canvas::outline_thickness(2.F)              //
                            | canvas::fill_color(sf::Color::Transparent)  //
                            | canvas::outline_color(sf::Color::Blue));
                        return true;
                    });
            }
            if (m.dcel)
            {
                m.dcel->faces()(
                    [&](const auto& face)
                    {
                        items.push_back(
                            canvas::polygon(face.as_polygon())            //
                            | canvas::outline_thickness(2.F)              //
                            | canvas::fill_color(sf::Color::Transparent)  //
                            | canvas::outline_color(sf::Color::White));
                        return true;
                    });
            }
            items.push_back(canvas::transform(
                [](const mx::vector_2d<float>& p) -> canvas::CanvasItem
                { return canvas::point(p, 5.F) | canvas::fill_color(sf::Color::Yellow); },
                m.points));

            return canvas::group(std::move(items));
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
        else if (const auto c = std::get_if<Commands::AddPoint>(&cmd))
        {
            m.points.push_back(c->pos);
            m.update();
            return {};
        }
        else if (const auto c = std::get_if<Commands::Init>(&cmd))
        {
            return {};
        }
        return {};
    };

    app.subscribe<InitEvent>([](Model& m, const InitEvent&) -> std::optional<Command> { return Commands::Init{}; });

    app.subscribe<TickEvent>([](Model& m, const TickEvent& event) -> std::optional<Command> { return {}; });
    app.subscribe<sf::Event::KeyPressed>(
        [](Model& m, const sf::Event::KeyPressed& e) -> std::optional<Command>
        {
            if (e.code == sf::Keyboard::Key::Escape)
            {
                return Commands::Exit{};
            }
            return {};
        });
    app.subscribe<sf::Event::MouseButtonPressed>(
        [](Model& m, const sf::Event::MouseButtonPressed& e) -> std::optional<Command>
        {
            if (e.button == sf::Mouse::Button::Left)
            {
                return Commands::AddPoint{ convert_as<float>(e.position) };
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
