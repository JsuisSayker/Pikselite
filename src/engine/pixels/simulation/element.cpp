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
    auto& def = g_elements[Element::WATER];

    if (tryMove(grid, x, y, x, y + GRAVITY_DIR)) return;

    int maxDisp = def.dispersionRate;

    int dir = (rand() % 2) ? -1 : 1;

    for (int d = 0; d < 2; ++d)
    {
        int dx = (d == 0) ? dir : -dir;
        for (int i = 1; i <= maxDisp; ++i)
        {
            int nx = x + dx * i;
            Element::Pixel& mid = grid.getPixelRef(x + dx * (i - 1), y);
            if (mid.type != Element::EMPTY && mid.type != Element::WATER)
                break;

            if (tryMove(grid, x, y, nx, y))
                return;

            if (tryMove(grid, x, y, nx, y + GRAVITY_DIR))
                return;
        }
    }
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

void updateDebug(ChunkGrid& grid, int x, int y)
{
}

void Simulation::initElements()
{
    g_elements[Element::EMPTY] = { "Empty", {0,0,0}, 0, SOLID, 0, nullptr, -1};
    g_elements[Element::SAND] = { "Sand", {194,178,128}, 5, SOLID, 1, updateSand, -1};
    g_elements[Element::WATER] = { "Water", {0,0,255}, 2, LIQUID, 5, updateWater, -1};
    g_elements[Element::FIRE] = { "Fire", {255,100,0}, 1, GAS, 1, updateFire, -1};
    g_elements[Element::STONE] = { "Stone", {100,100,100}, 255, SOLID, 0, updateStone, -1};
    g_elements[Element::DEBUG] = { "Debug", {255,0,255}, 1, SOLID, 0, updateDebug, -1};
}

void Simulation::update()
{
    frame++;
    resetUpdatedFlags();
    orderChunksForUpdate();
    detectRegions();

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

Element::Region Simulation::regionFloodFill(int x, int y, Element::ElementType type)
{
    Element::Region region;

    std::queue<Element::Vec2i> q;
    q.push({x, y});

    while (!q.empty())
    {
        auto [cx, cy] = q.front();
        q.pop();

        int64_t key = makeVisitedKey(cx, cy);
        if (visitedForRegions.count(key)) continue;
        visitedForRegions.insert(key);

        Element::Pixel p = grid.getPixel(cx, cy);
        if (p.type != type) continue;

        // set the pixels to debug for visualization
        // grid.setPixel(cx, cy, {Element::DEBUG, false});

        region.pixels.push_back({cx, cy});

        q.push({cx + 1, cy});
        q.push({cx - 1, cy});
        q.push({cx, cy + 1});
        q.push({cx, cy - 1});
    }

    return region;
}

void Simulation::detectRegions()
{
    visitedForRegions.clear();
    detectedRegions.clear();

    for (auto& [key, chunk] : grid.chunks)
    {
        int cx = static_cast<int32_t>(key >> 32);
        int cy = static_cast<int32_t>(key & 0xFFFFFFFF);

        for (int y = 0; y < CHUNK_SIZE; ++y)
        {
            for (int x = 0; x < CHUNK_SIZE; ++x)
            {
                Element::Pixel p = chunk.get(x, y);
                if (p.type == Element::EMPTY) continue;

                // only for stone for now
                if (p.type != Element::STONE) continue;

                int globalX = cx * CHUNK_SIZE + x;
                int globalY = cy * CHUNK_SIZE + y;

                if (visitedForRegions.count(makeVisitedKey(globalX, globalY))) continue;

                Element::Region region = regionFloodFill(globalX, globalY, p.type);
                detectedRegions.push_back(std::move(region));
            }
        }
    }
}
