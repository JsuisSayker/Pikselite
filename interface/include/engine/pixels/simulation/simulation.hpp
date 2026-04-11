#pragma once
#include <engine/pixels/simulation/element.hpp>
#include <algorithm>
#include <vector>
#include <queue>
#include <unordered_set>
#include <box2d/box2d.h>

class Simulation {
    public:
        struct BodyPixelBinding {
            Element::ElementType type;
            Element::Vec2f localUV;
            Element::Vec2f uv;
            int gridX;
            int gridY;
        };

        struct RegionBodyBinding {
            b2BodyId bodyId;
            std::vector<BodyPixelBinding> pixels;
        };

        Simulation(ChunkGrid &grid) : grid(grid) {
            initElements();
        }

        void initElements();
        void update();
        void resetUpdatedFlags();
        void setGrid(ChunkGrid &newGrid);
        ChunkGrid& getGrid() { return grid; }
        void markRegionsDirty() { regionsDirty = true; }
        void orderChunksForUpdate();
        void detectRegions();
        Element::Region regionFloodFill(int x, int y, Element::ElementType type);
        int64_t makeVisitedKey(int x, int y) {
            return (static_cast<int64_t>(x) << 32) | (static_cast<uint32_t>(y));
        }
        void buildRegionContoursMarchingSquare(Element::Region& region);
        void simplifyRegionContours(Element::Region& region, float epsilon);
        void triangulateRegion(Element::Region& region);
        const std::vector<Element::Region>& getDetectedRegions() const { return detectedRegions; }
        void setPhysicsWorld(b2WorldId worldId, float pixelsPerMeter);
        void rebuildRegionColliders();
        void syncBodyPixelsToGrid();
        const std::vector<b2BodyId>& getRegionBodies() const { return regionBodies; }
        const std::vector<RegionBodyBinding>& getRegionBodyBindings() const { return regionBodyBindings; }
        float getPixelsPerMeter() const { return pixelsPerMeter; }

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
        b2WorldId physicsWorld = b2_nullWorldId;
        float pixelsPerMeter = 1.0f;
        std::vector<b2BodyId> regionBodies;
        std::vector<RegionBodyBinding> regionBodyBindings;
        bool regionsDirty = true;
};
