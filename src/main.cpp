#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>
#include <deque>
#include <ferrugo/core/functional.hpp>
#include <ferrugo/core/sequence.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <variant>

#include "animation.hpp"
// #include "app_runner.hpp"
#include "canvas_item.hpp"
#include "event_handler.hpp"
#include "vec_t.hpp"

using namespace foo;

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

using duration_t = float;

struct TickEvent
{
    duration_t delta;
};

using Command = std::string;
// using Event = std::variant<sf::Event, TickEvent>;

struct View
{
    sf::RenderWindow& m_window;
    const sf::Font* m_font = nullptr;

    auto poll_event() -> std::optional<sf::Event>
    {
        sf::Event event{};
        if (m_window.pollEvent(event))
        {
            return std::optional<sf::Event>{ std::move(event) };
        }
        return std::optional<sf::Event>{};
    }

    auto is_open() const
    {
        return m_window.isOpen();
    }

    void close() const
    {
        m_window.close();
    }

    void handle_command(const Command& command)
    {
        if (!command.empty())
        {
            std::cout << "model -> view: " << command << "\n";
        }
        if (command == "exit")
        {
            m_window.close();
        }
        if (command == "no_font")
        {
            m_font = nullptr;
        }
    }

    void render(const CanvasItem& canvas_item)
    {
        m_window.clear();
        Context ctx{ m_window };
        State state{};
        state.text_style.font = m_font;
        canvas_item(ctx, state);
        m_window.display();
    }
};

template <class Model, class Event, class Command>
struct App
{
    using MaybeCommand = std::optional<Command>;
    using MaybeEvent = std::optional<Event>;
    using UpdateFn = std::function<std::tuple<Model, MaybeCommand>(const Model&, const Event&)>;
    using InitFn = std::function<MaybeCommand(const Model&)>;
    using EventHandlerFn = std::function<MaybeEvent(const std::variant<sf::Event, TickEvent>&)>;

    using CreateViewFn = std::function<CanvasItem(const Model&)>;

    Model m_model_state;
    View m_view;
    InitFn m_init;
    UpdateFn m_update;
    EventHandlerFn m_event_handler;
    CreateViewFn m_create_view;
    duration_t m_frame_duration = 0.01F;
    std::deque<Event> m_event_queue;

    void run()
    {
        sf::Clock clock;
        duration_t time_since_last_update = 0.F;

        const auto maybe_initial_command = m_init(m_model_state);
        if (maybe_initial_command)
        {
            m_view.handle_command(*maybe_initial_command);
        }

        while (m_view.is_open())
        {
            const duration_t elapsed = clock.restart().asSeconds();
            time_since_last_update += elapsed;

            // const fps_t fps = 1.0F / elapsed;

            while (time_since_last_update > m_frame_duration)
            {
                time_since_last_update -= m_frame_duration;

                while (std::optional<sf::Event> sf_event = m_view.poll_event())
                {
                    if (sf_event->type == sf::Event::Closed)
                    {
                        m_view.close();
                    }
                    if (const std::optional<Event> maybe_event = m_event_handler(*sf_event))
                    {
                        m_event_queue.push_back(*maybe_event);
                    }
                }

                if (const std::optional<Event> maybe_event = m_event_handler(TickEvent{ m_frame_duration }))
                {
                    m_event_queue.push_back(*maybe_event);
                }
            }

            while (!m_event_queue.empty())
            {
                Event event = m_event_queue.front();
                m_event_queue.pop_front();
                auto [new_model_state, maybe_command] = m_update(m_model_state, event);
                if (maybe_command)
                {
                    m_view.handle_command(*maybe_command);
                }
                m_model_state = std::move(new_model_state);
            }

            m_view.render(m_create_view(m_model_state));
        }
    }
};

struct Model
{
    float angle;
    float velocity;
};

void run()
{
    const sf::Font arial = load_font(R"(/mnt/c/Windows/Fonts/arial.ttf)");
    const sf::Font verdana = load_font(R"(/mnt/c/Windows/Fonts/verdana.ttf)");

    auto window = sf::RenderWindow{ { 1920u, 1080u }, "CMake SFML Project" };

    using namespace foo;

    const auto to_view = [](const Model& m) -> CanvasItem
    {
        using namespace ferrugo;
        return group(
            group(
                group(core::init(
                    15,
                    [](int x) -> CanvasItem
                    {
                        auto shape = x % 2 == 0  //
                                         ? circle(50.F)
                                         : rect(vec_t{ 100.F, 100.F });
                        return shape                                   //
                               | translate(vec_t{ 150.F * x, 500.F })  //
                               | color(sf::Color(200, 120, 140));
                    })),
                group(core::repeat(circle(50.F))  //
                          .take(3)                //
                          .transform_indexed(
                              [](int i, const CanvasItem& item) -> CanvasItem
                              {
                                  return item                              //
                                         | translate(vec_t{ 0, 125 } * i)  //
                                         | color(sf::Color::Red);
                              })),
                text("Hello")                  //
                    | font_size(48)            //
                    | color(sf::Color::White)  //
                    | translate(vec_t{ 150.F, 500.F }))
                | rotate(m.angle, vec_t{ 960.F, 540.F }),
            text(std::to_string(m.velocity)));
    };

    using Event = std::string;
    using Command = std::string;

    auto app = App<Model, Event, Command>{ Model{ 0.F, 0.F },
                                           View{ window, &arial },
                                           [](const Model& m) -> std::optional<Command> { return std::nullopt; },
                                           [](Model m, const Event& e) -> std::tuple<Model, std::optional<Command>>
                                           {
                                               if (e == "quit")
                                               {
                                                   return { m, Command{ "exit" } };
                                               }
                                               if (e == "left")
                                               {
                                                   m.velocity = 1;
                                                   return { m, std::nullopt };
                                               }
                                               if (e == "right")
                                               {
                                                   m.velocity = -1;
                                                   return { m, std::nullopt };
                                               }
                                               if (e == "tick")
                                               {
                                                   m.angle += m.velocity * 1.0;
                                               }
                                               return { m, std::nullopt };
                                           },
                                           [](const std::variant<sf::Event, TickEvent>& ev) -> std::optional<Event>
                                           {
                                               if (const auto e = std::get_if<sf::Event>(&ev))
                                               {
                                                   if (e->type == sf::Event::KeyPressed)
                                                   {
                                                       if (e->key.code == sf::Keyboard::Escape)
                                                       {
                                                           return Event{ "quit" };
                                                       }
                                                       if (e->key.code == sf::Keyboard::Left)
                                                       {
                                                           return Event{ "left" };
                                                       }
                                                       if (e->key.code == sf::Keyboard::Right)
                                                       {
                                                           return Event{ "right" };
                                                       }
                                                   }
                                               }
                                               if (const auto e = std::get_if<TickEvent>(&ev))
                                               {
                                                   return Event{ "tick" };
                                               }
                                               return std::nullopt;
                                           },
                                           to_view };

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
