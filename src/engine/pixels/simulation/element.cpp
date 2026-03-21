#include <engine/pixels/simulation/element/element.hpp>

ElementDefinition g_elements[256];

bool tryMove(ChunkGrid& grid, int x, int y, int nx, int ny)
    {
        simulation::Pixel src = grid.getPixel(x, y);
        simulation::Pixel dst = grid.getPixel(nx, ny);

        if (src.type == simulation::EMPTY) return false;

        auto& srcDef = g_elements[src.type];
        auto& dstDef = g_elements[dst.type];

        if (dst.type == simulation::EMPTY)
        {
            grid.setPixel(nx, ny, src);
            grid.setPixel(x, y, simulation::Pixel{simulation::EMPTY});
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
    g_elements[simulation::EMPTY] = {{0,0,0}, 0, SOLID, nullptr, -1};
    g_elements[simulation::SAND] = {{194,178,128}, 5, SOLID, updateSand, -1};
    g_elements[simulation::WATER] = {{0,0,255}, 2, LIQUID, updateWater, -1};
    g_elements[simulation::FIRE] = {{255,100,0}, 1, GAS, updateFire, -1};
    g_elements[simulation::STONE] = {{100,100,100}, 255, SOLID, updateStone, -1};
}
