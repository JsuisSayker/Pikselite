/*
** EPITECH PROJECT, 2025
** SDL2_POC
** File description:
** GraphicEnum
*/

#pragma once
#include <cstdint>

namespace graphic
{
    enum class EventType : uint8_t
    {
        WINDOW_CLOSE,
        MOUSE_CLICK_LEFT,
        MOUSE_CLICK_RIGHT,
        MOUSE_DRAG_LEFT,
        MOUSE_DRAG_RIGHT,
        MOUSE_MOVE,
        KEYBOARD_PRESS,
        KEY_ARROW_UP,
        KEY_ARROW_DOWN,
        KEY_ARROW_LEFT,
        KEY_ARROW_RIGHT,
        KEY_A,
        KEY_B,
        KEY_C,
        KEY_D,
        KEY_E,
        KEY_F,
        KEY_G,
        KEY_H,
        KEY_I,
        KEY_J,
        KEY_K,
        KEY_L,
        KEY_M,
        KEY_N,
        KEY_O,
        KEY_P,
        KEY_Q,
        KEY_R,
        KEY_S,
        KEY_T,
        KEY_U,
        KEY_V,
        KEY_W,
        KEY_X,
        KEY_Y,
        KEY_Z,
        NONE,
    };

    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    struct CameraPosition
    {
        float x;
        float y;
    };

    struct Camera
    {
        CameraPosition position;
        int zoom;
    };

    struct Position
    {
        float x;
        float y;
    };

    struct Pixel
    {
        Position position;
        Color color;
    };
}