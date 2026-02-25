#pragma once

#include <engine/pixels/pixelEnum.hpp>
#include <engine/pixels/chunk.hpp>

#include <algorithm>
#include <iostream>
#include <unordered_set>

namespace Pixel
{
    class PixelSimulation {
        public:
            PixelSimulation() = default;
            ~PixelSimulation() = default;

            void step(ChunkGrid& grid, PixelAttributes& attributes, std::vector<graphics::Pixel>& renderPixels, float deltaTime);
    
        private:
            ChunkGrid* _grid = nullptr;
            PixelAttributes* _attributes = nullptr;
            std::vector<graphics::Pixel>* _renderPixels = nullptr;

            float _elapsedTime = 0.0f;
            static constexpr float STEP_INTERVAL = 0.01f;
            bool _pixelSimulated = true;

            void simulateBottomUp();
            void liquidSimulation(int lx, int ly, PixelEntityID id, int cx, int cy);

            int64_t encodePos(int x, int y) const {
                return ((int64_t)(uint32_t)x << 32) | (uint32_t)y;
            }
    };
} // namespace Pixel