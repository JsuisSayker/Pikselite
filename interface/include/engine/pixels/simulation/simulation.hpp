#pragma once
#include <engine/pixels/simulation/element.hpp>

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

    private:
        ChunkGrid &grid;
};
