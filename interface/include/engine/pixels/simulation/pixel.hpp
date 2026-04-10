#pragma once
#include <cstdint>
#include <vector>

namespace Element {
    enum ElementType : uint16_t {
        EMPTY = 0,
        SAND,
        WATER,
        FIRE,
        STONE,
        DEBUG,
    };
    
    struct Pixel {
        ElementType type = EMPTY;
        bool updatedThisFrame = false;
    };

    struct Vec2i {
        int x, y;
    };

    struct Region {
        std::vector<Vec2i> pixels;
    };
}
