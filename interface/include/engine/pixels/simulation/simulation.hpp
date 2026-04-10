#pragma once
#include <engine/pixels/simulation/element.hpp>
#include <algorithm>
#include <vector>
#include <queue>
#include <unordered_set>

class Simulation {
    public:
        Simulation(ChunkGrid &grid) : grid(grid) {
            initElements();
        }

        void initElements();
        void update();
        void resetUpdatedFlags();
        void setGrid(ChunkGrid &newGrid) { grid = newGrid; }
        ChunkGrid& getGrid() { return grid; }
        void orderChunksForUpdate();
        void detectRegions();
        Element::Region regionFloodFill(int x, int y, Element::ElementType type);
        int64_t makeVisitedKey(int x, int y) {
            return (static_cast<int64_t>(x) << 32) | (static_cast<uint32_t>(y));
        }
        void buildRegionContoursMS(Element::Region& region);
        void simplifyRegionContours(Element::Region& region, float epsilon);
        void triangulateRegion(Element::Region& region);
        const std::vector<Element::Region>& getDetectedRegions() const { return detectedRegions; }

    private:
        uint64_t frame = 0;
        ChunkGrid &grid;

        struct ChunkEntry {
            int cx;
            int cy;
            Chunk* chunk;
        };

        std::vector<ChunkEntry> orderedChunks;
        std::vector<Element::Region> detectedRegions;

        std::unordered_set<int64_t> visitedForRegions;
};
