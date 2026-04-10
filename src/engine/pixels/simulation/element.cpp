#include <engine/pixels/simulation/element.hpp>
#include <engine/pixels/simulation/simulation.hpp>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <limits>

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

                // for demo, only detect stone regions. can be extended to other types or all types
                if (p.type != Element::STONE) continue;

                int globalX = cx * CHUNK_SIZE + x;
                int globalY = cy * CHUNK_SIZE + y;

                if (visitedForRegions.count(makeVisitedKey(globalX, globalY))) continue;

                Element::Region region = regionFloodFill(globalX, globalY, p.type);
                buildRegionContoursMS(region); // build edges here
                simplifyRegionContours(region, 0.25f);
                detectedRegions.push_back(std::move(region));
            }
        }
    }
}

void Simulation::buildRegionContoursMS(Element::Region& region)
{
    region.edges.clear();
    if (region.pixels.empty()) return;

    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;

    std::unordered_set<int64_t> occ;
    auto key = [](int x, int y) -> int64_t {
        return (static_cast<int64_t>(x) << 32) | static_cast<uint32_t>(y);
    };

    for (const auto& p : region.pixels) {
        minX = std::min(minX, p.x); minY = std::min(minY, p.y);
        maxX = std::max(maxX, p.x); maxY = std::max(maxY, p.y);
        occ.insert(key(p.x, p.y));
    }

    auto filled = [&](int x, int y) { return occ.count(key(x, y)) > 0; };
    auto P = [&](float x, float y) { return Element::Vec2f{x, y}; };

    for (int y = minY - 1; y <= maxY; ++y) {
        for (int x = minX - 1; x <= maxX; ++x) {
            bool bl = filled(x, y);
            bool br = filled(x + 1, y);
            bool tr = filled(x + 1, y + 1);
            bool tl = filled(x, y + 1);

            int c = (bl ? 1 : 0) | (br ? 2 : 0) | (tr ? 4 : 0) | (tl ? 8 : 0);
            if (c == 0 || c == 15) continue;

            Element::Vec2f L = P(x,       y + 0.5f);
            Element::Vec2f R = P(x + 1.0f,y + 0.5f);
            Element::Vec2f B = P(x + 0.5f,y);
            Element::Vec2f T = P(x + 0.5f,y + 1.0f);

            auto add = [&](Element::Vec2f a, Element::Vec2f b) {
                region.edges.push_back({a, b});
            };

            switch (c) {
                case 1:  add(L, B); break;
                case 2:  add(B, R); break;
                case 3:  add(L, R); break;
                case 4:  add(R, T); break;
                case 5:  add(L, T); add(B, R); break;
                case 6:  add(B, T); break;
                case 7:  add(L, T); break;
                case 8:  add(T, L); break;
                case 9:  add(T, B); break;
                case 10: add(T, R); add(L, B); break;
                case 11: add(T, R); break;
                case 12: add(R, L); break;
                case 13: add(B, R); break;
                case 14: add(L, B); break;
                default: break;
            }
        }
    }
}

namespace {
    struct IPoint {
        int x;
        int y;
    };

    int64_t makePointKey(int x, int y) {
        return (static_cast<int64_t>(x) << 32) | static_cast<uint32_t>(y);
    }

    float distPointToSegment(const Element::Vec2f& p, const Element::Vec2f& a, const Element::Vec2f& b) {
        const float vx = b.x - a.x;
        const float vy = b.y - a.y;
        const float wx = p.x - a.x;
        const float wy = p.y - a.y;

        const float c1 = vx * wx + vy * wy;
        if (c1 <= 0.0f) {
            const float dx = p.x - a.x;
            const float dy = p.y - a.y;
            return std::sqrt(dx * dx + dy * dy);
        }

        const float c2 = vx * vx + vy * vy;
        if (c2 <= c1) {
            const float dx = p.x - b.x;
            const float dy = p.y - b.y;
            return std::sqrt(dx * dx + dy * dy);
        }

        const float t = c1 / c2;
        const float projx = a.x + t * vx;
        const float projy = a.y + t * vy;
        const float dx = p.x - projx;
        const float dy = p.y - projy;
        return std::sqrt(dx * dx + dy * dy);
    }

    void rdpRecursive(const std::vector<Element::Vec2f>& pts, int start, int end, float eps, std::vector<bool>& keep) {
        if (end <= start + 1) return;

        float maxDist = -1.0f;
        int idx = -1;
        const Element::Vec2f& a = pts[start];
        const Element::Vec2f& b = pts[end];

        for (int i = start + 1; i < end; ++i) {
            float d = distPointToSegment(pts[i], a, b);
            if (d > maxDist) {
                maxDist = d;
                idx = i;
            }
        }

        if (maxDist > eps && idx != -1) {
            keep[idx] = true;
            rdpRecursive(pts, start, idx, eps, keep);
            rdpRecursive(pts, idx, end, eps, keep);
        }
    }

    std::vector<Element::Vec2f> rdpOpen(const std::vector<Element::Vec2f>& pts, float eps) {
        if (pts.size() <= 2) return pts;

        std::vector<bool> keep(pts.size(), false);
        keep.front() = true;
        keep.back() = true;

        rdpRecursive(pts, 0, static_cast<int>(pts.size() - 1), eps, keep);

        std::vector<Element::Vec2f> out;
        out.reserve(pts.size());
        for (size_t i = 0; i < pts.size(); ++i) {
            if (keep[i]) out.push_back(pts[i]);
        }
        return out;
    }

    bool samePoint(const Element::Vec2f& a, const Element::Vec2f& b) {
        return a.x == b.x && a.y == b.y;
    }
}

void Simulation::simplifyRegionContours(Element::Region& region, float epsilon)
{
    if (region.edges.empty()) return;

    struct Node {
        Element::Vec2f p;
        std::vector<int64_t> neighbors;
    };

    std::unordered_map<int64_t, Node> nodes;
    nodes.reserve(region.edges.size() * 2);

    auto quantize = [](const Element::Vec2f& p) -> IPoint {
        return {static_cast<int>(std::lround(p.x * 2.0f)), static_cast<int>(std::lround(p.y * 2.0f))};
    };

    auto toVec = [](const IPoint& ip) -> Element::Vec2f {
        return Element::Vec2f{ip.x / 2.0f, ip.y / 2.0f};
    };

    auto addNeighbor = [&](int64_t from, int64_t to) {
        nodes[from].neighbors.push_back(to);
    };

    for (const auto& seg : region.edges) {
        IPoint a = quantize(seg.a);
        IPoint b = quantize(seg.b);
        int64_t ka = makePointKey(a.x, a.y);
        int64_t kb = makePointKey(b.x, b.y);

        nodes[ka].p = toVec(a);
        nodes[kb].p = toVec(b);
        addNeighbor(ka, kb);
        addNeighbor(kb, ka);
    }

    std::unordered_set<uint64_t> visitedEdges;
    auto edgeKey = [](int64_t a, int64_t b) -> uint64_t {
        uint64_t ua = static_cast<uint64_t>(a);
        uint64_t ub = static_cast<uint64_t>(b);
        return (ua < ub) ? (ua << 32) ^ ub : (ub << 32) ^ ua;
    };

    std::vector<std::vector<Element::Vec2f>> loops;

    for (const auto& [startKey, node] : nodes) {
        for (int64_t nextKey : node.neighbors) {
            uint64_t ek = edgeKey(startKey, nextKey);
            if (visitedEdges.count(ek)) continue;

            std::vector<Element::Vec2f> loop;
            int64_t prev = startKey;
            int64_t curr = nextKey;

            loop.push_back(nodes[startKey].p);

            while (true) {
                visitedEdges.insert(edgeKey(prev, curr));
                loop.push_back(nodes[curr].p);

                if (curr == startKey) break;

                const auto& neigh = nodes[curr].neighbors;
                if (neigh.empty()) break;

                int64_t candidate = neigh.front();
                if (candidate == prev && neigh.size() > 1) candidate = neigh[1];

                prev = curr;
                curr = candidate;

                if (loop.size() > nodes.size() + 4) break;
            }

            if (loop.size() >= 4 && samePoint(loop.front(), loop.back())) {
                loops.push_back(std::move(loop));
            }
        }
    }

    std::vector<Element::Segment> simplifiedEdges;

    for (auto& loop : loops) {
        if (loop.size() < 4) continue;

        if (samePoint(loop.front(), loop.back())) {
            loop.pop_back();
        }

        auto simplified = rdpOpen(loop, epsilon);
        if (simplified.size() < 2) continue;

        simplified.push_back(simplified.front());
        for (size_t i = 0; i + 1 < simplified.size(); ++i) {
            simplifiedEdges.push_back({simplified[i], simplified[i + 1]});
        }
    }

    if (!simplifiedEdges.empty()) {
        region.edges = std::move(simplifiedEdges);
    }
}
