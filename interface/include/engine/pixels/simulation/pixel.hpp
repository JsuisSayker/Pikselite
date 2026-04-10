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

    struct Vec2i { int x, y; };
    struct Vec2f { float x, y; };

    struct Segment {
        Vec2f a, b;
    };

    struct Triangle {
        Vec2f a, b, c;
    };

    struct Region {
        std::vector<Vec2i> pixels;
        std::vector<Segment> edges; // marching-squares output
        std::vector<std::vector<Vec2f>> polygons; // simplified loops
        std::vector<Triangle> triangles; // triangulation output
    };
}
