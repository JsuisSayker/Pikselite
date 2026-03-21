#pragma once

#include <engine/pixels/simulation/element/chunk.hpp>
#include <cstdint>
#include <cstdlib>

enum ElementType : uint16_t {
    EMPTY = 0,
    SAND,
    WATER,
    FIRE,
    STONE,
};

enum ElementState : uint8_t {
    SOLID = 0,
    LIQUID,
    GAS,
};

struct Grid;

struct ElementDefinition {
    uint8_t color[3];
    uint8_t density;
    ElementState state;


    void (*update)(ChunkGrid& grid, int x, int y);

    // for future lua scripting to create custom behaviors
    int luaScriptID = -1;
};

ElementDefinition g_elements[256];

bool tryMove(ChunkGrid& grid, int x, int y, int nx, int ny)
    {
        Pixel src = grid.getPixel(x, y);
        Pixel dst = grid.getPixel(nx, ny);

        if (src.type == EMPTY) return false;

        auto& srcDef = g_elements[src.type];
        auto& dstDef = g_elements[dst.type];

        if (dst.type == EMPTY)
        {
            grid.setPixel(nx, ny, src);
            grid.setPixel(x, y, Pixel{EMPTY});
            return true;
        }

        if (srcDef.density > dstDef.density)
        {
            grid.setPixel(nx, ny, src);
            grid.setPixel(x, y, dst);
            return true;
        }

        return false;
    }

void updateWater(ChunkGrid& grid, int x, int y)
{
}

void updateSand(ChunkGrid& grid, int x, int y)
{
    if (tryMove(grid, x, y, x, y + 1)) return;

    if (rand() % 2)
    {
        if (tryMove(grid, x, y, x - 1, y + 1)) return;
        if (tryMove(grid, x, y, x + 1, y + 1)) return;
    }
    else
    {
        if (tryMove(grid, x, y, x + 1, y + 1)) return;
        if (tryMove(grid, x, y, x - 1, y + 1)) return;
    }
}
void updateFire(ChunkGrid& grid, int x, int y)
{
}
void updateStone(ChunkGrid& grid, int x, int y)
{
}

void initElements()
{
    g_elements[EMPTY] = {{0,0,0}, 0, SOLID, nullptr, -1};

    g_elements[SAND] = {{194,178,128}, 5, SOLID, updateSand, -1};

    g_elements[WATER] = {{0,0,255}, 2, LIQUID, updateWater, -1};

    g_elements[FIRE] = {{255,100,0}, 1, GAS, updateFire, -1};

    g_elements[STONE] = {{100,100,100}, 255, SOLID, updateStone, -1};
}

struct Pixel {
    ElementType type = EMPTY;
    bool updatedThisFrame = false;
};