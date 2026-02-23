#pragma once

#include <engine/pixels/pixelEnum.hpp>
#include <engine/pixels/chunk.hpp>

#include <iostream>
#include <algorithm>

namespace Pixel
{
    class PixelSimulation {
        public:
            PixelSimulation() = default;
            ~PixelSimulation() = default;

            void step(ChunkGrid& grid, PixelAttributes& attributes, std::vector<graphics::Pixel>& renderPixels, float deltaTime);
    
        private:
            // Member pointers (set at start of step())
            ChunkGrid* _grid = nullptr;
            PixelAttributes* _attributes = nullptr;
            std::vector<graphics::Pixel>* _renderPixels = nullptr;

            void simulateBottomUp(float deltaTime);
            void liquidSimulation(float deltaTime, int lx, int ly, PixelEntityID id, int cx, int cy);
    };
} // namespace Pixel