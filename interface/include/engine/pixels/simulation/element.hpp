#pragma once

#include <engine/pixels/simulation/chunk.hpp>
#include <cstdint>
#include <cstdlib>

constexpr int GRAVITY_DIR = -1;

enum ElementState : uint8_t {
    SOLID_STATIC = 0,
    SOLID_DYNAMIC,
    LIQUID,
    GAS,
};

struct ElementDefinition {
    std::string name;
    uint8_t color[3];
    uint8_t density;
    ElementState state;

    uint8_t dispersionRate;
    
    
    void (*update)(ChunkGrid& grid, int x, int y);
    
    // for future lua scripting to create custom behaviors
    int luaScriptID = -1;
};

extern ElementDefinition g_elements[256];

/**
 * @brief Attempts to move a pixel from one cell to another.
 * @param grid Simulation grid.
 * @param x Source global X.
 * @param y Source global Y.
 * @param nx Destination global X.
 * @param ny Destination global Y.
 * @return `true` if movement or swap occurred, otherwise `false`.
 */
bool tryMove(ChunkGrid& grid, int x, int y, int nx, int ny);

/**
 * @brief Updates one water pixel behavior.
 * @param grid Simulation grid.
 * @param x Global X coordinate.
 * @param y Global Y coordinate.
 * @return void
 */
void updateWater(ChunkGrid& grid, int x, int y);

/**
 * @brief Updates one sand pixel behavior.
 * @param grid Simulation grid.
 * @param x Global X coordinate.
 * @param y Global Y coordinate.
 * @return void
 */
void updateSand(ChunkGrid& grid, int x, int y);

/**
 * @brief Updates one fire pixel behavior.
 * @param grid Simulation grid.
 * @param x Global X coordinate.
 * @param y Global Y coordinate.
 * @return void
 */
void updateFire(ChunkGrid& grid, int x, int y);

/**
 * @brief Updates one stone pixel behavior.
 * @param grid Simulation grid.
 * @param x Global X coordinate.
 * @param y Global Y coordinate.
 * @return void
 */
void updateStone(ChunkGrid& grid, int x, int y);

/**
 * @brief Updates one dirt pixel behavior.
 * @param grid Simulation grid.
 * @param x Global X coordinate.
 * @param y Global Y coordinate.
 * @return void
 */
void updateDirt(ChunkGrid& grid, int x, int y);

/**
 * @brief Updates one debug pixel behavior.
 * @param grid Simulation grid.
 * @param x Global X coordinate.
 * @param y Global Y coordinate.
 * @return void
 */
void updateDebug(ChunkGrid& grid, int x, int y);
