#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window.hpp>

#include "defs.hpp"

struct event_handler_t
{
    template <class... Args>
    using handler_t = action_t<sf::RenderWindow&, Args...>;

    using size_handler_t = handler_t<sf::Event::SizeEvent>;
    using key_handler_t = handler_t<sf::Event::KeyEvent>;
    using mouse_button_handler_t = handler_t<sf::Event::MouseButtonEvent>;
    using mouse_wheel_scroll_handler_t = handler_t<sf::Event::MouseWheelScrollEvent>;
    using mouse_move_handler_t = handler_t<sf::Event::MouseMoveEvent>;
    using joystick_button_handler_t = handler_t<sf::Event::JoystickButtonEvent>;
    using joystick_move_handler_t = handler_t<sf::Event::JoystickMoveEvent>;
    using joystick_connect_handler_t = handler_t<sf::Event::JoystickConnectEvent>;
    using touch_handler_t = handler_t<sf::Event::TouchEvent>;
    using text_handler_t = handler_t<sf::Event::TextEvent>;
    using sensor_handler_t = handler_t<sf::Event::SensorEvent>;

    handler_t<> on_close;
    handler_t<> on_focus_lost;
    handler_t<> on_focus_gained;
    handler_t<> on_mouse_entered;
    handler_t<> on_mouse_left;

    size_handler_t on_resized;

    key_handler_t on_key_pressed;
    key_handler_t on_key_released;

    mouse_button_handler_t on_mouse_button_pressed;
    mouse_button_handler_t on_mouse_button_released;

    mouse_wheel_scroll_handler_t on_mouse_wheel_scroll;

    mouse_move_handler_t on_mouse_moved;

    joystick_button_handler_t on_joystick_button_pressed;
    joystick_button_handler_t on_joystick_button_released;

    joystick_move_handler_t on_joystick_moved;

    joystick_connect_handler_t on_joystick_connected;
    joystick_connect_handler_t on_joystick_disconnected;

    touch_handler_t on_touch_began;
    touch_handler_t on_touch_ended;
    touch_handler_t on_touch_moved;

    text_handler_t on_text_entered;

    sensor_handler_t on_sensor_changed;

    void operator()(sf::RenderWindow& window, const sf::Event& event) const
    {
        switch (event.type)
        {
            case sf::Event::Closed: return invoke(window, on_close);
            case sf::Event::LostFocus: return invoke(window, on_focus_lost);
            case sf::Event::GainedFocus: return invoke(window, on_focus_gained);
            case sf::Event::MouseEntered: return invoke(window, on_mouse_entered);
            case sf::Event::MouseLeft: return invoke(window, on_mouse_left);
            case sf::Event::Resized: return invoke(window, on_resized, event.size);
            case sf::Event::KeyPressed: return invoke(window, on_key_pressed, event.key);
            case sf::Event::KeyReleased: return invoke(window, on_key_released, event.key);
            case sf::Event::MouseButtonPressed: return invoke(window, on_mouse_button_pressed, event.mouseButton);
            case sf::Event::MouseButtonReleased: return invoke(window, on_mouse_button_released, event.mouseButton);
            case sf::Event::MouseMoved: return invoke(window, on_mouse_moved, event.mouseMove);
            case sf::Event::MouseWheelScrolled: return invoke(window, on_mouse_wheel_scroll, event.mouseWheelScroll);
            case sf::Event::JoystickButtonPressed: return invoke(window, on_joystick_button_pressed, event.joystickButton);
            case sf::Event::JoystickButtonReleased: return invoke(window, on_joystick_button_released, event.joystickButton);
            case sf::Event::JoystickMoved: return invoke(window, on_joystick_moved, event.joystickMove);
            case sf::Event::JoystickConnected: return invoke(window, on_joystick_connected, event.joystickConnect);
            case sf::Event::JoystickDisconnected: return invoke(window, on_joystick_disconnected, event.joystickConnect);
            case sf::Event::TouchBegan: return invoke(window, on_touch_began, event.touch);
            case sf::Event::TouchEnded: return invoke(window, on_touch_ended, event.touch);
            case sf::Event::TouchMoved: return invoke(window, on_touch_moved, event.touch);
            case sf::Event::TextEntered: return invoke(window, on_text_entered, event.text);
            case sf::Event::SensorChanged: return invoke(window, on_sensor_changed, event.sensor);
            default: break;
        }
    }

    template <class Handler, class... Args>
    void invoke(sf::RenderWindow& window, Handler& handler, Args&&... arg) const
    {
        if (handler)
        {
            handler(window, std::forward<Args>(arg)...);
        }
    }
};