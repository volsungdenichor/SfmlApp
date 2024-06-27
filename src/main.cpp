#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <functional>
#include <iostream>
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

void run_app(
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

struct vec_t : std::array<float, 2>
{
    using base_t = std::array<float, 2>;

    vec_t(float x, float y) : base_t{ { x, y } }
    {
    }

    vec_t(const sf::Vector2f& v) : vec_t(v.x, v.y)
    {
    }

    vec_t() : vec_t(0.F, 0.F)
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

    friend vec_t operator+(const vec_t& item)
    {
        return item;
    }

    friend vec_t operator-(vec_t item)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            item[i] = -item[i];
        }
        return item;
    }

    friend vec_t& operator*=(vec_t& lhs, float rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] *= rhs;
        }
        return lhs;
    }

    friend vec_t operator*(vec_t lhs, float rhs)
    {
        return lhs *= rhs;
    }

    friend vec_t& operator/=(vec_t& lhs, float rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] /= rhs;
        }
        return lhs;
    }

    friend vec_t operator/(vec_t lhs, float rhs)
    {
        return lhs /= rhs;
    }

    friend vec_t operator*(float lhs, const vec_t& rhs)
    {
        return rhs * lhs;
    }

    friend vec_t& operator+=(vec_t& lhs, const vec_t& rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] += rhs[i];
        }
        return lhs;
    }

    friend vec_t operator+(vec_t lhs, const vec_t& rhs)
    {
        return lhs += rhs;
    }

    friend vec_t& operator-=(vec_t& lhs, const vec_t& rhs)
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            lhs[i] -= rhs[i];
        }
        return lhs;
    }

    friend vec_t operator-(vec_t lhs, const vec_t& rhs)
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
    virtual void set_geometry(
        const applier_t<vec_t>& position, const applier_t<vec_t>& scale, const applier_t<float>& rotation)
        = 0;

    virtual void set_style(
        const applier_t<sf::Color>& fill_color,
        const applier_t<sf::Color>& outline_color,
        const applier_t<float>& outline_thickness)
        = 0;

    virtual void set_texture(const sf::Texture* texture, const sf::IntRect& rect) = 0;

    virtual void set_text(const applier_t<sf::String>& text, const sf::Font& font) = 0;
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

    void set_geometry(
        const applier_t<vec_t>& position, const applier_t<vec_t>& scale, const applier_t<float>& rotation) override
    {
        {
            vec_t v(m_inner.getPosition());
            position(v);
            m_inner.setPosition(v);
        }
        {
            vec_t v(m_inner.getScale());
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

    void set_texture(const sf::Texture* texture, const sf::IntRect& rect) override
    {
        m_inner.setTexture(texture);
        m_inner.setTextureRect(rect);
    }

    void set_text(const applier_t<sf::String>& text, const sf::Font& font) override
    {
    }
};

struct sprite_widget : widget_impl
{
    sf::Sprite m_inner;

    explicit sprite_widget(sf::Sprite inner = {}) : m_inner(std::move(inner))
    {
    }

    std::unique_ptr<widget_impl> clone() const
    {
        return std::make_unique<sprite_widget>(m_inner);
    }

    void draw(sf::RenderTarget& window, sf::RenderStates states) const
    {
        window.draw(m_inner, states);
    }

    void set_geometry(
        const applier_t<vec_t>& position, const applier_t<vec_t>& scale, const applier_t<float>& rotation) override
    {
        {
            vec_t v(m_inner.getPosition());
            position(v);
            m_inner.setPosition(v);
        }
        {
            vec_t v(m_inner.getScale());
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
    }

    void set_texture(const sf::Texture* texture, const sf::IntRect& rect)
    {
        m_inner.setTexture(*texture);
        m_inner.setTextureRect(rect);
    }

    void set_text(const applier_t<sf::String>& text, const sf::Font& font) override
    {
    }
};

struct text_widget : widget_impl
{
    sf::Text m_inner;

    explicit text_widget(sf::Text inner = {}) : m_inner(std::move(inner))
    {
    }

    std::unique_ptr<widget_impl> clone() const
    {
        return std::make_unique<text_widget>(m_inner);
    }

    void draw(sf::RenderTarget& window, sf::RenderStates states) const
    {
        window.draw(m_inner, states);
    }

    void set_geometry(
        const applier_t<vec_t>& position, const applier_t<vec_t>& scale, const applier_t<float>& rotation) override
    {
        {
            vec_t v(m_inner.getPosition());
            position(v);
            m_inner.setPosition(v);
        }
        {
            vec_t v(m_inner.getScale());
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

    void set_texture(const sf::Texture* texture, const sf::IntRect& rect)
    {
    }

    void set_text(const applier_t<sf::String>& text, const sf::Font& font) override
    {
        sf::String v = m_inner.getString();
        text(v);
        m_inner.setString(v);
        m_inner.setFont(font);
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

    void set_geometry(const applier_t<vec_t>& position, const applier_t<vec_t>& scale, const applier_t<float>& rotation)
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

    void set_texture(const sf::Texture* texture, const sf::IntRect& rect)
    {
        m_ptr->set_texture(texture, rect);
    }

    void set_text(const applier_t<sf::String>& text, const sf::Font& font)
    {
        m_ptr->set_text(text, font);
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

auto polygon(const std::vector<vec_t>& v) -> widget_t
{
    sf::ConvexShape sh(v.size());
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        sh.setPoint(i, v[i]);
    }

    return widget_t::create<shape_widget<sf::ConvexShape>>(std::move(sh));
}

auto sprite(const sf::Texture& texture, const sf::IntRect& rect) -> widget_t
{
    widget_t result = widget_t::create<sprite_widget>();
    result.set_texture(&texture, rect);
    return result;
}

auto text(const sf::String& str, const sf::Font& font)
{
    widget_t result = widget_t::create<text_widget>();
    result.set_text(set_value(str), font);
    return result;
}

auto operator|(widget_t lhs, widget_modifier_t rhs) -> widget_t
{
    rhs(lhs);
    return lhs;
}

auto position(vec_t v) -> widget_modifier_t
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

auto texture(const sf::Texture* texture, const sf::IntRect& rect)
{
    return [=](widget_t& w) { w.set_texture(texture, rect); };
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
    const sf::Font fnt = load_font(R"(/mnt/c/Windows/Fonts/arial.ttf)");

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
        [&](sf::RenderWindow& w)
        {
            std::vector<drawable> drawables;
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
                text("Hello World!", fnt) | outline(sf::Color::White) | fill(sf::Color::Red) | outline_thickness(1.F)
                | position({ 1200, 400 }));

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