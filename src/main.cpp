#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <functional>
#include <memory>

struct do_nothing_fn
{
    template <class... Args>
    constexpr void operator()(Args&&...) const
    {
    }
};

static constexpr inline auto do_nothing = do_nothing_fn{};

template <class... Args>
using action_t = std::function<void(Args...)>;

template <class T>
using applier_t = action_t<T&>;

void run(
    sf::RenderWindow& window,
    const action_t<const sf::Event&>& event_handler,
    const action_t<float>& updater,
    const applier_t<sf::RenderWindow>& renderer,
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

    vec(const sf::Vector2f& v) : vec(v.x, v.y)
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

struct widget_impl
{
    using ptr = std::unique_ptr<widget_impl>;

    virtual ~widget_impl() = default;
    virtual std::unique_ptr<widget_impl> clone() const = 0;
    virtual void draw(sf::RenderTarget& window, sf::RenderStates states) const = 0;
    virtual void set_geometry(const applier_t<vec>& position, const applier_t<vec>& scale, const applier_t<float>& rotation)
        = 0;

    virtual void set_style(
        const applier_t<sf::Color>& fill_color,
        const applier_t<sf::Color>& outline_color,
        const applier_t<float>& outline_thickness)
        = 0;
};

template <class S>
struct shape_widget : widget_impl
{
    S m_inner;

    explicit shape_widget(S inner) : m_inner(std::move(inner))
    {
    }

    std::unique_ptr<widget_impl> clone() const override
    {
        return std::make_unique<shape_widget<S>>(m_inner);
    }

    void draw(sf::RenderTarget& window, sf::RenderStates states) const override
    {
        window.draw(m_inner, states);
    }

    void set_geometry(const applier_t<vec>& position, const applier_t<vec>& scale, const applier_t<float>& rotation) override
    {
        {
            vec v(m_inner.getPosition());
            position(v);
            m_inner.setPosition(v);
        }
        {
            vec v(m_inner.getScale());
            scale(v);
            m_inner.setScale(v);
        }
        {
            float r = m_inner.getRotation();
            rotation(r);
            m_inner.setRotation(r);
        }
    }

    void set_style(
        const applier_t<sf::Color>& fill_color,
        const applier_t<sf::Color>& outline_color,
        const applier_t<float>& outline_thickness) override
    {
        {
            sf::Color v = m_inner.getFillColor();
            fill_color(v);
            m_inner.setFillColor(v);
        }
        {
            sf::Color v = m_inner.getOutlineColor();
            outline_color(v);
            m_inner.setOutlineColor(v);
        }
        {
            float v = m_inner.getOutlineThickness();
            outline_thickness(v);
            m_inner.setOutlineThickness(v);
        }
    }
};

struct widget_t
{
    widget_impl::ptr m_ptr;

    explicit widget_t(widget_impl::ptr ptr) : m_ptr(std::move(ptr))
    {
    }

    widget_t(widget_t&&) = default;

    widget_t(const widget_t& other) : widget_t(m_ptr->clone())
    {
    }

    template <class T, class... Args>
    static widget_t create(Args&&... args)
    {
        return widget_t(std::make_unique<T>(std::forward<Args>(args)...));
    }

    widget_t& operator=(widget_t other)
    {
        std::swap(m_ptr, other.m_ptr);
        return *this;
    }

    void draw(sf::RenderTarget& window, sf::RenderStates states) const
    {
        m_ptr->draw(window, std::move(states));
    }

    void operator()(sf::RenderTarget& window) const
    {
        draw(window, sf::RenderStates::Default);
    }

    void set_geometry(const applier_t<vec>& position, const applier_t<vec>& scale, const applier_t<float>& rotation)
    {
        m_ptr->set_geometry(position, scale, rotation);
    }

    void set_style(
        const applier_t<sf::Color>& fill_color,
        const applier_t<sf::Color>& outline_color,
        const applier_t<float>& outline_thickness)
    {
        m_ptr->set_style(fill_color, outline_color, outline_thickness);
    }
};

template <class T>
auto set_value(T v) -> applier_t<T>
{
    return [=](T& out) { out = v; };
}

using drawable = action_t<sf::RenderTarget&>;
using widget_modifier_t = action_t<widget_t&>;

auto rect(float w, float h) -> widget_t
{
    return widget_t::create<shape_widget<sf::RectangleShape>>(sf::RectangleShape(sf::Vector2f(w, h)));
}

auto circle(float r) -> widget_t
{
    return widget_t::create<shape_widget<sf::CircleShape>>(sf::CircleShape(r));
}

auto polygon(const std::vector<vec>& v) -> widget_t
{
    sf::ConvexShape sh(v.size());
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        sh.setPoint(i, v[i]);
    }

    return widget_t::create<shape_widget<sf::ConvexShape>>(std::move(sh));
}

auto operator|(widget_t lhs, widget_modifier_t rhs) -> widget_t
{
    rhs(lhs);
    return lhs;
}

auto position(vec v) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_geometry(set_value(std::move(v)), do_nothing, do_nothing); };
}

auto fill(sf::Color color) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_style(set_value(color), do_nothing, do_nothing); };
}

auto outline(sf::Color color) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_style(do_nothing, set_value(color), do_nothing); };
}

auto outline_thickness(float v) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_style(do_nothing, do_nothing, set_value(v)); };
}

auto rotate(float a) -> widget_modifier_t
{
    return [=](widget_t& w) { w.set_geometry(do_nothing, do_nothing, set_value(a)); };
}

auto operator|(widget_modifier_t lhs, widget_modifier_t rhs) -> widget_modifier_t
{
    return [=](widget_t& sh)
    {
        lhs(sh);
        rhs(sh);
    };
}

template <class... Args>
auto all(Args&&... args) -> widget_modifier_t
{
    std::vector<widget_modifier_t> modifiers{ std::forward<Args>(args)... };
    return [=](widget_t& w)
    {
        for (const auto& m : modifiers)
        {
            m(w);
        }
    };
}

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
    float angle = 0.F;
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
            angle += dt * 50.F;
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
            std::vector<drawable> drawables;
            drawables.push_back(circle(200) | fill(sf::Color::Yellow) | outline(sf::Color::Red) | outline_thickness(5.0));
            drawables.push_back(
                rect(50.F, 20.F) | fill(sf::Color::Red) | outline(sf::Color::Yellow) | position({ 100, 50 }) | rotate(45.F));
            drawables.push_back(circle(50.F) | fill(sf::Color::Green) | position({ 200, 50 }));
            drawables.push_back(
                polygon({ vec(0, 0),   vec(20, 0),  vec(30, 10), vec(50, 10), vec(60, 0),  vec(80, 0),  vec(80, 20),
                          vec(70, 30), vec(70, 50), vec(80, 60), vec(80, 80), vec(60, 80), vec(50, 70), vec(30, 70),
                          vec(20, 80), vec(0, 80),  vec(0, 60),  vec(10, 50), vec(10, 30), vec(0, 20) })
                | position({ 200, 200 }) | rotate(angle) | fill(sf::Color::Red));
            for (const auto& d : drawables)
            {
                d(w);
            }
        },
        0.01F);
}
