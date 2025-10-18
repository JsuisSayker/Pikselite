#pragma once

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1030

#define PIXEL_SIZE 10.0f

namespace graphics {
    struct Coord {
        float x, y;
    };

    struct Pixel {
        float x, y;
        float r, g, b;
    };

    enum InputEventType {
        MOUSE_LEFT_CLICK,
        QUIT,
        NO_EVENT,
    };
}