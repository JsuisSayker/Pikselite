#pragma once

#include <engine/pixels/simulation/element/chunk.hpp>
#include <cstdint>
#include <cstdlib>

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

bool tryMove(ChunkGrid& grid, int x, int y, int nx, int ny);
void updateWater(ChunkGrid& grid, int x, int y);
void updateSand(ChunkGrid& grid, int x, int y);
void updateFire(ChunkGrid& grid, int x, int y);
void updateStone(ChunkGrid& grid, int x, int y);
void initElements();