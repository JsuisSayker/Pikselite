#pragma once
#include <engine/pixels/simulation/element.hpp>
#include <algorithm>
#include <vector>

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

    private:
        uint64_t frame = 0;
        ChunkGrid &grid;

        struct ChunkEntry {
            int cx;
            int cy;
            Chunk* chunk;
        };

        std::vector<ChunkEntry> orderedChunks;
};
