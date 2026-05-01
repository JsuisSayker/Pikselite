#pragma once
#include <cstdint>
#include <vector>

namespace Element
{
    /**
     * @brief Supported element identifiers used by the simulation grid.
     */
    enum ElementType : uint16_t
    {
        /** @brief Empty cell. */
        EMPTY = 0,
        /** @brief Sand element. */
        SAND,
        /** @brief Water element. */
        WATER,
        /** @brief Fire element. */
        FIRE,
        /** @brief Stone element. */
        STONE,
        /** @brief Dirt element. */
        DIRT,
        /** @brief Debug-only element. */
        DEBUG,
    };

    /**
     * @brief One simulation cell payload.
     */
    struct Pixel
    {
        /** @brief Element kind stored in this cell. */
        ElementType type = EMPTY;
        /** @brief Marks whether this pixel was already processed this frame. */
        bool updatedThisFrame = false;
    };

    /**
     * @brief Integer 2D coordinate.
     */
    struct Vec2i
    {
        int x, y;
    };

    /**
     * @brief Floating-point 2D coordinate.
     */
    struct Vec2f
    {
        float x, y;
    };

    /**
     * @brief 2D line segment.
     */
    struct Segment
    {
        /** @brief Segment start point. */
        Vec2f a, b;
    };

    /**
     * @brief 2D triangle primitive.
     */
    struct Triangle
    {
        /** @brief Triangle vertices. */
        Vec2f a, b, c;
    };

    /**
     * @brief Region extraction and geometry buffers.
     */
    struct Region
    {
        /** @brief Grid pixels that belong to this connected region. */
        std::vector<Vec2i> pixels;
        /** @brief Marching-squares contour segments. */
        std::vector<Segment> edges; // marching-squares output
        /** @brief Simplified contour loops. */
        std::vector<std::vector<Vec2f>> polygons; // simplified loops
        /** @brief Triangulation output of simplified loops. */
        std::vector<Triangle> triangles; // triangulation output
    };
} // namespace Element
