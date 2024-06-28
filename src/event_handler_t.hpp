#pragma once

#include <SFML/Window.hpp>

#include "defs.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

struct event_handler_t
{
    template <class... Args>
    using handler_t = action_t<sf::RenderWindow&, Args...>;

    handler_t<> on_close;
    handler_t<> on_focus_lost;
    handler_t<> on_focus_gained;
    handler_t<> on_mouse_entered;
    handler_t<> on_mouse_left;
    handler_t<sf::Event::SizeEvent> on_resized;
    handler_t<sf::Event::KeyEvent> on_key_pressed;
    handler_t<sf::Event::KeyEvent> on_key_released;
    handler_t<sf::Event::MouseButtonEvent> on_mouse_button_pressed;
    handler_t<sf::Event::MouseButtonEvent> on_mouse_button_released;
    handler_t<sf::Event::MouseWheelScrollEvent> on_mouse_wheel_scroll;
    handler_t<sf::Event::MouseMoveEvent> on_mouse_moved;
    handler_t<sf::Event::JoystickButtonEvent> on_joystick_button_pressed;
    handler_t<sf::Event::JoystickButtonEvent> on_joystick_button_released;
    handler_t<sf::Event::JoystickMoveEvent> on_joystick_moved;
    handler_t<sf::Event::JoystickConnectEvent> on_joystick_connected;
    handler_t<sf::Event::JoystickConnectEvent> on_joystick_disconnected;
    handler_t<sf::Event::TouchEvent> on_touch_began;
    handler_t<sf::Event::TouchEvent> on_touch_ended;
    handler_t<sf::Event::TouchEvent> on_touch_moved;
    handler_t<sf::Event::TextEvent> on_text_entered;
    handler_t<sf::Event::SensorEvent> on_sensor_changed;

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