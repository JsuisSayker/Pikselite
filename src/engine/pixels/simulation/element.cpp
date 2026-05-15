#include <box2d/box2d.h>
#include <cmath>
#include <engine/pixels/simulation/element.hpp>
#include <engine/pixels/simulation/simulation.hpp>
#include <limits>
#include <tracy/Tracy.hpp>
#include <unordered_map>
#include <unordered_set>

#ifndef TRACY_ENABLE
// output a warning if profiling is disabled
#pragma message(                                                                                   \
    "Tracy profiling is disabled. To enable, set PIKSELITE_ENABLE_PROFILING=ON in CMake and rebuild.")
#error "Not set"
#endif

ElementDefinition g_elements[256] = {};

namespace
{
    bool g_stoneRegionsDirtyThisFrame = false;
}

bool tryMove(ChunkGrid& grid, int x, int y, int nx, int ny)
{
    ZoneScopedN("Sim::tryMove");
    Element::Pixel& src = grid.getPixelRef(x, y);
    Element::Pixel& dst = grid.getPixelRef(nx, ny);

    if (src.type == Element::EMPTY)
        return false;

    auto& srcDef = g_elements[src.type];
    auto& dstDef = g_elements[dst.type];

    if (dst.type == Element::EMPTY)
    {
        dst                  = src;
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
    ZoneScopedN("Sim::Water");
    ElementDefinition& def = g_elements[Element::WATER];

    // lava interaction
    // Check for lava below water in a range for more natural interaction
    int interactionRange = 3;
    for (int i = 1; i <= interactionRange; ++i)
    {
        Element::Pixel& below = grid.getPixelRef(x, y - i);
        if (below.type == Element::LAVA)
        {
            below.type                   = Element::STONE;
            below.updatedThisFrame       = true;
            g_stoneRegionsDirtyThisFrame = true;

            grid.setPixel(x, y, {Element::FIRE, false});
            return;
        }
    }

    if (tryMove(grid, x, y, x, y + GRAVITY_DIR))
        return;

    int maxDisp = def.dispersionRate;

    int dir = (rand() % 2) ? -1 : 1;

    for (int d = 0; d < 2; ++d)
    {
        int dx = (d == 0) ? dir : -dir;
        for (int i = 1; i <= maxDisp; ++i)
        {
            int             nx  = x + dx * i;
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
void updateLava(ChunkGrid& grid, int x, int y)
{
    ElementDefinition& def = g_elements[Element::LAVA];

    if (rand() % 255 < def.fireParams.burnSpreadChance / 5)
    {
        Element::Pixel& p = grid.getPixelRef(x, y + 1);
        if (p.type == Element::EMPTY)
        {
            p.type             = Element::FIRE;
            p.burnTimer        = g_elements[Element::FIRE].fireParams.burnDuration;
            p.updatedThisFrame = true;
        }
    }

    if (tryMove(grid, x, y, x, y + GRAVITY_DIR))
        return;

    int maxDisp = def.dispersionRate;

    int dir = (rand() % 2) ? -1 : 1;

    for (int d = 0; d < 2; ++d)
    {
        int dx = (d == 0) ? dir : -dir;
        for (int i = 1; i <= maxDisp; ++i)
        {
            int             nx  = x + dx * i;
            Element::Pixel& mid = grid.getPixelRef(x + dx * (i - 1), y);
            if (mid.type != Element::EMPTY && mid.type != Element::LAVA)
                break;

            if (tryMove(grid, x, y, nx, y))
                break;

            if (tryMove(grid, x, y, nx, y + GRAVITY_DIR))
                break;
        }
    }

    const int dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

    for (auto& d : dirs)
    {
        int nx = x + d[0];
        int ny = y + d[1];

        Element::Pixel& neighbor = grid.getPixelRef(nx, ny);
        if (neighbor.type == Element::EMPTY || neighbor.isBurning)
            continue;

        ElementDefinition& nDef = g_elements[neighbor.type];
        if (nDef.fireParams.flammability == 0)
            continue;

        uint16_t chance = (nDef.fireParams.flammability * def.fireParams.burnSpreadChance) / 255;
        if (rand() % 255 < chance)
        {
            neighbor.isBurning        = true;
            neighbor.burnTimer        = nDef.fireParams.burnDuration;
            neighbor.updatedThisFrame = true;
        }
    }
}
void updateSand(ChunkGrid& grid, int x, int y)
{
    ZoneScopedN("Sim::Sand");
    if (tryMove(grid, x, y, x, y + GRAVITY_DIR))
        return;

    if (rand() % 2)
    {
        if (tryMove(grid, x, y, x - 1, y + GRAVITY_DIR))
            return;
        if (tryMove(grid, x, y, x + 1, y + GRAVITY_DIR))
            return;
    }
    else
    {
        if (tryMove(grid, x, y, x + 1, y + GRAVITY_DIR))
            return;
        if (tryMove(grid, x, y, x - 1, y + GRAVITY_DIR))
            return;
    }
}
void updateFire(ChunkGrid& grid, int x, int y)
{
    ZoneScopedN("Sim::Fire");
    Element::Pixel&    p   = grid.getPixelRef(x, y);
    ElementDefinition& def = g_elements[Element::FIRE];

    if (p.burnTimer > 0)
        p.burnTimer--;
    else
    {
        p = Element::Pixel{Element::EMPTY};
        return;
    }

    const int dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

    for (auto& d : dirs)
    {
        int nx = x + d[0];
        int ny = y + d[1];

        Element::Pixel& neighbor = grid.getPixelRef(nx, ny);
        if (neighbor.type == Element::EMPTY || neighbor.isBurning)
            continue;

        ElementDefinition& nDef = g_elements[neighbor.type];
        if (nDef.fireParams.flammability == 0)
            continue;

        uint16_t chance = (nDef.fireParams.flammability * def.fireParams.burnSpreadChance) / 255;
        if (rand() % 255 < chance)
        {
            neighbor.isBurning        = true;
            neighbor.burnTimer        = nDef.fireParams.burnDuration;
            neighbor.updatedThisFrame = true;
        }
    }

    if (tryMove(grid, x, y, x, y - GRAVITY_DIR))
        return;
    int dir = (rand() % 2) ? -1 : 1;
    if (tryMove(grid, x, y, x + dir, y - GRAVITY_DIR))
        return;
    if (tryMove(grid, x, y, x - dir, y - GRAVITY_DIR))
        return;
}
void updateStone(ChunkGrid& grid, int x, int y) {}
void updateDirt(ChunkGrid& grid, int x, int y) {}
void updateDebug(ChunkGrid& grid, int x, int y) {}

void Simulation::initElements()
{
    g_elements[Element::EMPTY] = {"Empty", {}, 0, SOLID_STATIC, 0, fireBehavior{}, nullptr, -1};
    g_elements[Element::SAND]  = {"Sand", {}, 5, SOLID_DYNAMIC, 1, fireBehavior{}, updateSand, -1};
    g_elements[Element::WATER] = {"Water", {}, 2, LIQUID, 5, fireBehavior{}, updateWater, -1};
    g_elements[Element::LAVA]  = {"Lava", {}, 3, LIQUID, 2, fireBehavior{}, updateLava, -1};
    g_elements[Element::FIRE]  = {"Fire", {}, 1, GAS, 1, fireBehavior{}, updateFire, -1};
    g_elements[Element::STONE] = {"Stone",        {},          255, SOLID_STATIC, 0,
                                  fireBehavior{}, updateStone, -1};
    g_elements[Element::DIRT]  = {"Dirt", {}, 10, SOLID_STATIC, 1, fireBehavior{}, updateDirt, -1};
    g_elements[Element::WOOD]  = {
        "Wood", {}, 5, SOLID_STATIC, 1, fireBehavior{150, 20, 30, Element::FIRE}, nullptr, -1};
    g_elements[Element::DEBUG] = {"Debug", {}, 1, SOLID_STATIC, 0, fireBehavior{}, updateDebug, -1};

    g_elements[Element::SAND].colorPalette = {
        {{194, 178, 128}, {206, 188, 140}, {182, 164, 116}, {216, 198, 150}}};
    g_elements[Element::WATER].colorPalette = {
        {{30, 90, 200}, {50, 120, 220}, {70, 150, 240}, {20, 70, 180}}};
    g_elements[Element::LAVA].colorPalette = {
        {{255, 50, 0}, {255, 100, 0}, {255, 150, 50}, {200, 30, 0}}};
    g_elements[Element::FIRE].colorPalette = {
        {{255, 80, 0}, {255, 120, 0}, {255, 180, 50}, {200, 40, 0}}};
    g_elements[Element::STONE].colorPalette = {
        {{95, 95, 100}, {75, 75, 80}, {115, 115, 120}, {60, 60, 65}}};
    g_elements[Element::DIRT].colorPalette = {
        {{110, 75, 40}, {130, 90, 50}, {90, 60, 30}, {70, 45, 20}}};
    g_elements[Element::WOOD].colorPalette = {
        {{120, 70, 15}, {140, 90, 30}, {100, 50, 10}, {160, 110, 50}}};
    g_elements[Element::DEBUG].colorPalette = {
        {{255, 0, 255}, {200, 0, 200}, {150, 0, 150}, {255, 100, 255}}};

    g_elements[Element::LAVA].fireParams = {0, 0, 50, Element::STONE};
    g_elements[Element::FIRE].fireParams = {0, 6, 50, Element::EMPTY};
    g_elements[Element::WOOD].fireParams = {150, 30, 30, Element::EMPTY};
}

inline void Simulation::updateBurning(ChunkGrid& grid, int x, int y)
{
    ZoneScopedN("Sim::Burning");
    Element::Pixel&    p          = grid.getPixelRef(x, y);
    ElementDefinition& elementDef = g_elements[p.type];

    if (p.burnTimer > 0)
        p.burnTimer--;
    else
    {
        const Element::ElementType previousType = p.type;
        p.type                                  = elementDef.fireParams.burnToElement;
        p.isBurning                             = false;
        if (previousType == Element::STONE || p.type == Element::STONE)
        {
            g_stoneRegionsDirtyThisFrame = true;
        }
    }

    const int dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

    for (auto& d : dirs)
    {
        int nx = x + d[0];
        int ny = y + d[1];

        Element::Pixel& n = grid.getPixelRef(nx, ny);
        if (n.type == Element::EMPTY || n.isBurning)
            continue;
        ElementDefinition& nDef = g_elements[n.type];
        if (nDef.fireParams.flammability == 0)
            continue;

        uint16_t chance =
            (nDef.fireParams.flammability * elementDef.fireParams.burnSpreadChance) / 255;
        if (rand() % 255 < chance)
        {
            n.isBurning        = true;
            n.burnTimer        = nDef.fireParams.burnDuration;
            n.updatedThisFrame = true;
        }
    }

    if (rand() % 255 < elementDef.fireParams.burnSpreadChance / 5)
    {
        Element::Pixel& p = grid.getPixelRef(x, y + 1);
        if (p.type != Element::EMPTY)
            return;
        p.type             = Element::FIRE;
        p.burnTimer        = g_elements[Element::FIRE].fireParams.burnDuration;
        p.updatedThisFrame = true;
    }
}

void Simulation::updateParticles()
{
    for (auto it = particles.begin(); it != particles.end();)
    {
        if (it->lifetime == 0)
        {
            it = particles.erase(it);
            continue;
        }

        it->lifetime--;
        {
            // simple Euler integration, can be improved with substepping or Verlet if needed
            it->position.x += it->velocity.x;
            it->position.y += it->velocity.y;

            // apply gravity
            it->velocity.y -= 0.1f; // gravity strength, can be tuned

            // apply some damping to velocity
            it->velocity.x *= 0.98f;
            it->velocity.y *= 0.98f;

            // check for collisions with the grid and update velocity accordingly
            int            gridX  = static_cast<int>(it->position.x);
            int            gridY  = static_cast<int>(it->position.y);
            Element::Pixel p      = grid.getPixel(gridX, gridY);
            bool           erased = false;
            if (p.type == Element::EMPTY)
            {
                // check if there is neighboring pixel
                const int dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
                for (auto& d : dirs)
                {
                    int            nx       = gridX + d[0];
                    int            ny       = gridY + d[1];
                    Element::Pixel neighbor = grid.getPixel(nx, ny);
                    if (neighbor.type != Element::EMPTY)
                    {
                        // reinsert the particle in the grid and stop its movement
                        grid.setPixel(gridX, gridY, {it->type, false, 0, 0, false});
                        it     = particles.erase(it);
                        erased = true;
                        break;
                    }
                }
            }
            if (!erased)
            {
                ++it;
            }
        }
    }
}

void Simulation::update()
{
    frame++;
    updateParticles();
    resetUpdatedFlags();
    orderChunksForUpdate();

    for (auto& entry : orderedChunks)
    {
        int    cx    = entry.cx;
        int    cy    = entry.cy;
        Chunk& chunk = *entry.chunk;

        bool flip = ((frame + cy) % 2 == 0);

        // bottom -> top for GRAVITY_DIR = -1
        for (int y = 0; y < CHUNK_SIZE; ++y)
        {
            for (int x = flip ? 0 : CHUNK_SIZE - 1; flip ? x < CHUNK_SIZE : x >= 0;
                 flip ? ++x : --x)
            {
                Element::Pixel& p = chunk.get(x, y);

                if (p.type == Element::EMPTY || p.updatedThisFrame)
                    continue;

                p.updatedThisFrame = true;

                if (p.isBurning)
                    updateBurning(grid, cx * CHUNK_SIZE + x, cy * CHUNK_SIZE + y);
                else
                {
                    ElementDefinition& def = g_elements[p.type];
                    if (def.update)
                        def.update(grid, cx * CHUNK_SIZE + x, cy * CHUNK_SIZE + y);
                }
            }
        }
    }

    if (g_stoneRegionsDirtyThisFrame)
    {
        regionsDirty                 = true;
        g_stoneRegionsDirtyThisFrame = false;
    }

    if (regionsDirty)
    {
        detectRegions();
    }
}

void Simulation::spawnParticle(Element::ElementType type, Element::Vec2f position,
                               Element::Vec2f velocity, uint8_t colorIndex, uint16_t lifetime)
{
    particles.push_back(Element::Particle{position, velocity, type, colorIndex, lifetime});
}

void Simulation::setGrid(ChunkGrid& newGrid)
{
    if (b2World_IsValid(physicsWorld))
    {
        for (b2BodyId bodyId : regionBodies)
        {
            if (b2Body_IsValid(bodyId))
                b2DestroyBody(bodyId);
        }
    }

    grid = newGrid;
    detectedRegions.clear();
    regionBodies.clear();
    regionBodyBindings.clear();
    regionsDirty = true;
}

void Simulation::setPhysicsWorld(b2WorldId worldId, float pixelsPerMeterValue)
{
    physicsWorld   = worldId;
    pixelsPerMeter = pixelsPerMeterValue;
    regionsDirty   = true;
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

bool Simulation::tryDisplacePixel(int x, int y, int range)
{
    Element::Pixel existing = grid.getPixel(x, y);
    if (existing.type == Element::EMPTY)
        return false;

    auto tryPlace = [&](int nx, int ny) -> bool
    {
        Element::Pixel& dst = grid.getPixelRef(nx, ny);
        if (dst.type != Element::EMPTY)
            return false;

        Element::Pixel moved   = existing;
        moved.updatedThisFrame = true;
        grid.setPixel(x, y, {Element::EMPTY, false});
        dst = moved;
        return true;
    };

    for (int i = 1; i <= range; ++i)
    {
        if (tryPlace(x, y + GRAVITY_DIR * i))
            return true;
    }

    for (int i = 1; i <= range; ++i)
    {
        if (tryPlace(x - i, y))
            return true;
        if (tryPlace(x + i, y))
            return true;
    }

    for (int i = 1; i <= range; ++i)
    {
        if (tryPlace(x, y - GRAVITY_DIR * i))
            return true;
    }

    return false;
}

void Simulation::detectRegions()
{
    if (!b2World_IsValid(physicsWorld))
    {
        regionsDirty = false;
        return;
    }

    for (b2BodyId bodyId : regionBodies)
    {
        if (b2Body_IsValid(bodyId))
            b2DestroyBody(bodyId);
    }

    detectedRegions.clear();
    regionBodies.clear();
    regionBodyBindings.clear();
    visitedForRegions.clear();

    for (const auto& [key, chunk] : grid.chunks)
    {
        const int cx = static_cast<int32_t>(key >> 32);
        const int cy = static_cast<int32_t>(key & 0xFFFFFFFF);

        for (int y = 0; y < CHUNK_SIZE; ++y)
        {
            for (int x = 0; x < CHUNK_SIZE; ++x)
            {
                const Element::Pixel& pixel = chunk.pixels[y * CHUNK_SIZE + x];
                if (pixel.type != Element::STONE)
                    continue;

                const int     gx         = cx * CHUNK_SIZE + x;
                const int     gy         = cy * CHUNK_SIZE + y;
                const int64_t visitedKey = makeVisitedKey(gx, gy);
                if (visitedForRegions.count(visitedKey))
                    continue;

                Element::Region region = regionFloodFill(gx, gy, Element::STONE);
                if (region.pixels.empty())
                    continue;

                buildRegionContoursMarchingSquare(region);
                simplifyRegionContours(region, 0.6f);
                triangulateRegion(region);

                if (region.triangles.empty())
                    continue;

                b2BodyDef bodyDef = b2DefaultBodyDef();
                bodyDef.type      = b2_staticBody;
                bodyDef.position  = {0.0f, 0.0f};

                b2BodyId   bodyId             = b2CreateBody(physicsWorld, &bodyDef);
                b2ShapeDef shapeDef           = b2DefaultShapeDef();
                shapeDef.material.friction    = 0.8f;
                shapeDef.material.restitution = 0.0f;

                bool createdShape = false;
                for (const auto& tri : region.triangles)
                {
                    b2Vec2 points[3] = {
                        {tri.a.x * pixelsPerMeter, tri.a.y * pixelsPerMeter},
                        {tri.b.x * pixelsPerMeter, tri.b.y * pixelsPerMeter},
                        {tri.c.x * pixelsPerMeter, tri.c.y * pixelsPerMeter},
                    };

                    b2Hull hull = b2ComputeHull(points, 3);
                    if (hull.count == 3)
                    {
                        b2Polygon polygon = b2MakePolygon(&hull, 0.0f);
                        b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
                        createdShape = true;
                    }
                }

                if (!createdShape)
                {
                    if (b2Body_IsValid(bodyId))
                        b2DestroyBody(bodyId);
                    continue;
                }

                detectedRegions.push_back(std::move(region));
                regionBodies.push_back(bodyId);
            }
        }
    }

    regionsDirty = false;
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
        if (visitedForRegions.count(key))
            continue;
        visitedForRegions.insert(key);

        Element::Pixel p = grid.getPixel(cx, cy);
        if (p.type != type)
            continue;

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

void Simulation::buildRegionContoursMarchingSquare(Element::Region& region)
{
    region.edges.clear();
    if (region.pixels.empty())
        return;

    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;

    std::unordered_set<int64_t> occ;
    auto                        key = [](int x, int y) -> int64_t
    { return (static_cast<int64_t>(x) << 32) | static_cast<uint32_t>(y); };

    for (const auto& p : region.pixels)
    {
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
        occ.insert(key(p.x, p.y));
    }

    auto filled = [&](int x, int y) { return occ.count(key(x, y)) > 0; };
    auto P      = [&](float x, float y) { return Element::Vec2f{x, y}; };

    for (int y = minY - 1; y <= maxY; ++y)
    {
        for (int x = minX - 1; x <= maxX; ++x)
        {
            bool bl = filled(x, y);
            bool br = filled(x + 1, y);
            bool tr = filled(x + 1, y + 1);
            bool tl = filled(x, y + 1);

            int c = (bl ? 1 : 0) | (br ? 2 : 0) | (tr ? 4 : 0) | (tl ? 8 : 0);
            if (c == 0 || c == 15)
                continue;

            Element::Vec2f L = P(x, y + 0.5f);
            Element::Vec2f R = P(x + 1.0f, y + 0.5f);
            Element::Vec2f B = P(x + 0.5f, y);
            Element::Vec2f T = P(x + 0.5f, y + 1.0f);

            auto add = [&](Element::Vec2f a, Element::Vec2f b) { region.edges.push_back({a, b}); };

            switch (c)
            {
                case 1:
                    add(L, B);
                    break;
                case 2:
                    add(B, R);
                    break;
                case 3:
                    add(L, R);
                    break;
                case 4:
                    add(R, T);
                    break;
                case 5:
                    add(L, T);
                    add(B, R);
                    break;
                case 6:
                    add(B, T);
                    break;
                case 7:
                    add(L, T);
                    break;
                case 8:
                    add(T, L);
                    break;
                case 9:
                    add(T, B);
                    break;
                case 10:
                    add(T, R);
                    add(L, B);
                    break;
                case 11:
                    add(T, R);
                    break;
                case 12:
                    add(R, L);
                    break;
                case 13:
                    add(B, R);
                    break;
                case 14:
                    add(L, B);
                    break;
                default:
                    break;
            }
        }
    }
}

namespace
{
    struct IPoint
    {
        int x;
        int y;
    };

    int64_t makePointKey(int x, int y)
    {
        return (static_cast<int64_t>(x) << 32) | static_cast<uint32_t>(y);
    }

    float distPointToSegment(const Element::Vec2f& p, const Element::Vec2f& a,
                             const Element::Vec2f& b)
    {
        const float vx = b.x - a.x;
        const float vy = b.y - a.y;
        const float wx = p.x - a.x;
        const float wy = p.y - a.y;

        const float c1 = vx * wx + vy * wy;
        if (c1 <= 0.0f)
        {
            const float dx = p.x - a.x;
            const float dy = p.y - a.y;
            return std::sqrt(dx * dx + dy * dy);
        }

        const float c2 = vx * vx + vy * vy;
        if (c2 <= c1)
        {
            const float dx = p.x - b.x;
            const float dy = p.y - b.y;
            return std::sqrt(dx * dx + dy * dy);
        }

        const float t     = c1 / c2;
        const float projx = a.x + t * vx;
        const float projy = a.y + t * vy;
        const float dx    = p.x - projx;
        const float dy    = p.y - projy;
        return std::sqrt(dx * dx + dy * dy);
    }

    void rdpRecursive(const std::vector<Element::Vec2f>& pts, int start, int end, float eps,
                      std::vector<bool>& keep)
    {
        if (end <= start + 1)
            return;

        float                 maxDist = -1.0f;
        int                   idx     = -1;
        const Element::Vec2f& a       = pts[start];
        const Element::Vec2f& b       = pts[end];

        for (int i = start + 1; i < end; ++i)
        {
            float d = distPointToSegment(pts[i], a, b);
            if (d > maxDist)
            {
                maxDist = d;
                idx     = i;
            }
        }

        if (maxDist > eps && idx != -1)
        {
            keep[idx] = true;
            rdpRecursive(pts, start, idx, eps, keep);
            rdpRecursive(pts, idx, end, eps, keep);
        }
    }

    std::vector<Element::Vec2f> rdpOpen(const std::vector<Element::Vec2f>& pts, float eps)
    {
        if (pts.size() <= 2)
            return pts;

        std::vector<bool> keep(pts.size(), false);
        keep.front() = true;
        keep.back()  = true;

        rdpRecursive(pts, 0, static_cast<int>(pts.size() - 1), eps, keep);

        std::vector<Element::Vec2f> out;
        out.reserve(pts.size());
        for (size_t i = 0; i < pts.size(); ++i)
        {
            if (keep[i])
                out.push_back(pts[i]);
        }
        return out;
    }

    bool samePoint(const Element::Vec2f& a, const Element::Vec2f& b)
    {
        return a.x == b.x && a.y == b.y;
    }
} // namespace

void Simulation::simplifyRegionContours(Element::Region& region, float epsilon)
{
    region.polygons.clear();
    if (region.edges.empty())
        return;

    struct Node
    {
        Element::Vec2f       p;
        std::vector<int64_t> neighbors;
    };

    std::unordered_map<int64_t, Node> nodes;
    nodes.reserve(region.edges.size() * 2);

    auto quantize = [](const Element::Vec2f& p) -> IPoint
    {
        return {static_cast<int>(std::lround(p.x * 2.0f)),
                static_cast<int>(std::lround(p.y * 2.0f))};
    };

    auto toVec = [](const IPoint& ip) -> Element::Vec2f
    { return Element::Vec2f{ip.x / 2.0f, ip.y / 2.0f}; };

    auto addNeighbor = [&](int64_t from, int64_t to) { nodes[from].neighbors.push_back(to); };

    for (const auto& seg : region.edges)
    {
        IPoint  a  = quantize(seg.a);
        IPoint  b  = quantize(seg.b);
        int64_t ka = makePointKey(a.x, a.y);
        int64_t kb = makePointKey(b.x, b.y);

        nodes[ka].p = toVec(a);
        nodes[kb].p = toVec(b);
        addNeighbor(ka, kb);
        addNeighbor(kb, ka);
    }

    std::unordered_set<uint64_t> visitedEdges;
    auto                         edgeKey = [](int64_t a, int64_t b) -> uint64_t
    {
        uint64_t ua = static_cast<uint64_t>(a);
        uint64_t ub = static_cast<uint64_t>(b);
        return (ua < ub) ? (ua << 32) ^ ub : (ub << 32) ^ ua;
    };

    std::vector<std::vector<Element::Vec2f>> loops;

    for (const auto& [startKey, node] : nodes)
    {
        for (int64_t nextKey : node.neighbors)
        {
            uint64_t ek = edgeKey(startKey, nextKey);
            if (visitedEdges.count(ek))
                continue;

            std::vector<Element::Vec2f> loop;
            int64_t                     prev = startKey;
            int64_t                     curr = nextKey;

            loop.push_back(nodes[startKey].p);

            while (true)
            {
                visitedEdges.insert(edgeKey(prev, curr));
                loop.push_back(nodes[curr].p);

                if (curr == startKey)
                    break;

                const auto& neigh = nodes[curr].neighbors;
                if (neigh.empty())
                    break;

                int64_t candidate = neigh.front();
                if (candidate == prev && neigh.size() > 1)
                    candidate = neigh[1];

                prev = curr;
                curr = candidate;

                if (loop.size() > nodes.size() + 4)
                    break;
            }

            if (loop.size() >= 4 && samePoint(loop.front(), loop.back()))
            {
                loops.push_back(std::move(loop));
            }
        }
    }

    std::vector<Element::Segment> simplifiedEdges;

    for (auto& loop : loops)
    {
        if (loop.size() < 4)
            continue;

        if (samePoint(loop.front(), loop.back()))
        {
            loop.pop_back();
        }

        auto simplified = rdpOpen(loop, epsilon);
        if (simplified.size() < 2)
            continue;

        region.polygons.push_back(simplified);

        simplified.push_back(simplified.front());
        for (size_t i = 0; i + 1 < simplified.size(); ++i)
        {
            simplifiedEdges.push_back({simplified[i], simplified[i + 1]});
        }
    }

    if (!simplifiedEdges.empty())
    {
        region.edges = std::move(simplifiedEdges);
    }
}

namespace
{
    float polygonArea(const std::vector<Element::Vec2f>& poly)
    {
        float area = 0.0f;
        for (size_t i = 0; i < poly.size(); ++i)
        {
            const auto& a = poly[i];
            const auto& b = poly[(i + 1) % poly.size()];
            area += a.x * b.y - b.x * a.y;
        }
        return area * 0.5f;
    }

    bool isPointInTri(const Element::Vec2f& p, const Element::Vec2f& a, const Element::Vec2f& b,
                      const Element::Vec2f& c)
    {
        auto cross = [](const Element::Vec2f& u, const Element::Vec2f& v)
        { return u.x * v.y - u.y * v.x; };

        Element::Vec2f v0{c.x - a.x, c.y - a.y};
        Element::Vec2f v1{b.x - a.x, b.y - a.y};
        Element::Vec2f v2{p.x - a.x, p.y - a.y};

        float den = cross(v1, v0);
        if (std::fabs(den) < 1e-6f)
            return false;

        float u = cross(v2, v0) / den;
        float v = cross(v1, v2) / den;
        return (u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f);
    }

    bool isConvex(const Element::Vec2f& prev, const Element::Vec2f& curr,
                  const Element::Vec2f& next)
    {
        float cross = (curr.x - prev.x) * (next.y - curr.y) - (curr.y - prev.y) * (next.x - curr.x);
        return cross > 0.0f;
    }

    std::vector<Element::Vec2f> pruneCollinear(const std::vector<Element::Vec2f>& poly, float eps)
    {
        if (poly.size() < 3)
            return poly;

        std::vector<Element::Vec2f> out;
        out.reserve(poly.size());

        auto area2 = [](const Element::Vec2f& a, const Element::Vec2f& b, const Element::Vec2f& c)
        { return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x); };

        const size_t n = poly.size();
        for (size_t i = 0; i < n; ++i)
        {
            const Element::Vec2f& prev = poly[(i + n - 1) % n];
            const Element::Vec2f& curr = poly[i];
            const Element::Vec2f& next = poly[(i + 1) % n];

            float a2 = std::fabs(area2(prev, curr, next));
            if (a2 <= eps)
                continue;
            out.push_back(curr);
        }

        return out;
    }
} // namespace

void Simulation::triangulateRegion(Element::Region& region)
{
    region.triangles.clear();
    if (region.polygons.empty())
        return;

    for (auto poly : region.polygons)
    {
        if (poly.size() < 3)
            continue;

        poly = pruneCollinear(poly, 1e-4f);
        if (poly.size() < 3)
            continue;

        if (polygonArea(poly) < 0.0f)
        {
            std::reverse(poly.begin(), poly.end());
        }

        std::vector<int> idx(poly.size());
        for (size_t i = 0; i < poly.size(); ++i)
            idx[i] = static_cast<int>(i);

        int guard = 0;
        while (idx.size() > 2 && guard++ < 10000)
        {
            bool earFound = false;
            for (size_t i = 0; i < idx.size(); ++i)
            {
                int i0 = idx[(i + idx.size() - 1) % idx.size()];
                int i1 = idx[i];
                int i2 = idx[(i + 1) % idx.size()];

                const auto& a = poly[i0];
                const auto& b = poly[i1];
                const auto& c = poly[i2];

                if (!isConvex(a, b, c))
                    continue;

                bool anyInside = false;
                for (size_t j = 0; j < idx.size(); ++j)
                {
                    int v = idx[j];
                    if (v == i0 || v == i1 || v == i2)
                        continue;
                    if (isPointInTri(poly[v], a, b, c))
                    {
                        anyInside = true;
                        break;
                    }
                }

                if (anyInside)
                    continue;

                region.triangles.push_back({a, b, c});
                idx.erase(idx.begin() + static_cast<int>(i));
                earFound = true;
                break;
            }

            if (!earFound)
                break;
        }
    }
}