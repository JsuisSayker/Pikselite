#include <engine/pixels/simulation/element.hpp>
#include <engine/pixels/simulation/simulation.hpp>

ElementDefinition g_elements[256] = {};

bool tryMove(ChunkGrid& grid, int x, int y, int nx, int ny)
{
    Element::Pixel& src = grid.getPixelRef(x, y);
    Element::Pixel& dst = grid.getPixelRef(nx, ny);

    if (src.type == Element::EMPTY) return false;

    auto& srcDef = g_elements[src.type];
    auto& dstDef = g_elements[dst.type];

    if (dst.type == Element::EMPTY)
    {
        dst = src;
        dst.updatedThisFrame = true;

        src = Element::Pixel{Element::EMPTY};
        return true;
    }

    if (srcDef.density > dstDef.density)
    {
        std::swap(src, dst);

        dst.updatedThisFrame = true;
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
    g_elements[Element::EMPTY] = { "Empty", {0,0,0}, 0, SOLID, nullptr, -1};
    g_elements[Element::SAND] = { "Sand", {194,178,128}, 5, SOLID, updateSand, -1};
    g_elements[Element::WATER] = { "Water", {0,0,255}, 2, LIQUID, updateWater, -1};
    g_elements[Element::FIRE] = { "Fire", {255,100,0}, 1, GAS, updateFire, -1};
    g_elements[Element::STONE] = { "Stone", {100,100,100}, 255, SOLID, updateStone, -1};
}

void Simulation::update()
{
    frame++;
    resetUpdatedFlags();
    orderChunksForUpdate();

    for (auto& entry : orderedChunks)
    {
        int cx = entry.cx;
        int cy = entry.cy;
        Chunk& chunk = *entry.chunk;

        bool flip = ((frame + cy) % 2 == 0);

        // bottom -> top for GRAVITY_DIR = -1
        for (int y = 0; y < CHUNK_SIZE; ++y)
        {
            for (int x = flip ? 0 : CHUNK_SIZE - 1;
                 flip ? x < CHUNK_SIZE : x >= 0;
                 flip ? ++x : --x)
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

void Simulation::orderChunksForUpdate()
{
    orderedChunks.clear();
    for (auto& [key, chunk] : grid.chunks)
    {
        int cx = static_cast<int32_t>(key >> 32);
        int cy = static_cast<int32_t>(key & 0xFFFFFFFF);

        orderedChunks.push_back({cx, cy, &chunk});
    }

    std::sort(orderedChunks.begin(), orderedChunks.end(),
        [](const ChunkEntry& a, const ChunkEntry& b)
        {
            if (a.cy != b.cy)
                return a.cy < b.cy; // bottom -> top for GRAVITY_DIR = -1
            return a.cx < b.cx;     // left -> right
        });
}
