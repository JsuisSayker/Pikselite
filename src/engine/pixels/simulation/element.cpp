#include <engine/pixels/simulation/element.hpp>
#include <engine/pixels/simulation/simulation.hpp>

ElementDefinition g_elements[256] = {};

bool tryMove(ChunkGrid& grid, int x, int y, int nx, int ny)
{
    Element::Pixel src = grid.getPixel(x, y);
    Element::Pixel dst = grid.getPixel(nx, ny);

    if (src.type == Element::EMPTY) return false;

    auto& srcDef = g_elements[src.type];
    auto& dstDef = g_elements[dst.type];

    if (dst.type == Element::EMPTY)
    {
        grid.setPixel(nx, ny, src);
        grid.getPixelRef(nx, ny).updatedThisFrame = true;

        grid.setPixel(x, y, Element::Pixel{Element::EMPTY});
        return true;
    }

    if (srcDef.density > dstDef.density)
    {
        grid.setPixel(nx, ny, src);
        grid.getPixelRef(nx, ny).updatedThisFrame = true;

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
    if (tryMove(grid, x, y, x, y + GRAVITY_DIR)) return;

    if (rand() % 2)
    {
        if (tryMove(grid, x, y, x - 1, y + GRAVITY_DIR)) return;
        if (tryMove(grid, x, y, x + 1, y + GRAVITY_DIR)) return;
    }
    else
    {
        if (tryMove(grid, x, y, x + 1, y + GRAVITY_DIR)) return;
        if (tryMove(grid, x, y, x - 1, y + GRAVITY_DIR)) return;
    }
}
void updateFire(ChunkGrid& grid, int x, int y)
{
}
void updateStone(ChunkGrid& grid, int x, int y)
{
}

void Simulation::initElements()
{
    g_elements[Element::EMPTY] = {{0,0,0}, 0, SOLID, nullptr, -1};
    g_elements[Element::SAND] = {{194,178,128}, 5, SOLID, updateSand, -1};
    g_elements[Element::WATER] = {{0,0,255}, 2, LIQUID, updateWater, -1};
    g_elements[Element::FIRE] = {{255,100,0}, 1, GAS, updateFire, -1};
    g_elements[Element::STONE] = {{100,100,100}, 255, SOLID, updateStone, -1};
}

void Simulation::update()
{
    resetUpdatedFlags();
    for (auto& [key, chunk] : grid.chunks)
    {
        int cx = key >> 32;
        int cy = key & 0xFFFFFFFF;

        for (int y = CHUNK_SIZE - 1; y >= 0; --y)
        {
            for (int x = 0; x < CHUNK_SIZE; ++x)
            {
                Element::Pixel& p = chunk.get(x, y);

                if (p.type == Element::EMPTY || p.updatedThisFrame)
                    continue;

                p.updatedThisFrame = true;

                auto& def = g_elements[p.type];
                if (def.update)
                    def.update(grid, cx * CHUNK_SIZE + x, cy * CHUNK_SIZE + y);
            }
        }
    }
}

void Simulation::resetUpdatedFlags()
{
    for (auto& [key, chunk] : grid.chunks)
    {
        for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i)
        {
            chunk.pixels[i].updatedThisFrame = false;
        }
    }
}
