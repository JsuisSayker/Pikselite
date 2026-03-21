#pragma once
#include <cstdint>

namespace simulation {
    enum ElementType : uint16_t {
        EMPTY = 0,
        SAND,
        WATER,
        FIRE,
        STONE,
    };
    
    struct Pixel {
        ElementType type = EMPTY;
        bool updatedThisFrame = false;
    };
}
    