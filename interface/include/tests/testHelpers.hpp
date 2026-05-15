#pragma once
#include <chrono>
#include <engine/pixels/simulation/element.hpp>
#include <gtest/gtest.h>

struct TimedScope
{
    std::chrono::high_resolution_clock::time_point start;
    const char* name;
    double maxSeconds;

    TimedScope(const char* name_, double maxSec)
        : start(std::chrono::high_resolution_clock::now())
        , name(name_)
        , maxSeconds(maxSec)
    {
    }

    ~TimedScope()
    {
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();
        EXPECT_LT(elapsed, maxSeconds) << name << " exceeded time limit of " << maxSeconds << "s";
    }
};

inline void fillRect(ChunkGrid& grid, int x, int y, int w, int h, Element::ElementType type)
{
    Element::Pixel p;
    p.type = type;
    for (int j = y; j < y + h; ++j)
        for (int i = x; i < x + w; ++i)
            grid.setPixel(i, j, p);
}

inline void placePixelG(ChunkGrid& grid, int x, int y, Element::ElementType type)
{
    Element::Pixel p;
    p.type = type;
    grid.setPixel(x, y, p);
}
