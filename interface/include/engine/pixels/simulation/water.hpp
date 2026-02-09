#pragma once

#include <engine/pixels/pixelEnum.hpp>
#include <engine/pixels/chunk.hpp>


namespace Pixel {
    struct IPixelSystem {
        virtual void collectPixels(Pixel::ChunkGrid& grid, Pixel::PixelAttributes& attrs, std::vector<graphics::Pixel>& renderPixels) = 0;
        virtual void step(Pixel::ChunkGrid& grid, Pixel::PixelAttributes& attrs, std::vector<graphics::Pixel>& renderPixels, float deltaTime) = 0;
        virtual ~IPixelSystem() = default;
    };

    class WaterSystem : public IPixelSystem {
    public:
        WaterSystem() = default;
        WaterSystem(const WaterSystem&) = delete;

        void collectPixels(Pixel::ChunkGrid& grid, Pixel::PixelAttributes& attrs, std::vector<graphics::Pixel>& renderPixels) override {
            activeWater.clear();
            // collect coordinates of all water pixels
            for (auto& kv : grid.getChunks()) {
                auto cx = kv.first.first;
                auto cy = kv.first.second;
                auto& chunk = kv.second;

                for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                    for (int ly = 0; ly < CHUNK_SIZE; ++ly) {
                        auto id = chunk.get(lx, ly);
                        if (id != Pixel::EMPTY && attrs.liquidAttributes.count(id)) {
                            // Convert local chunk coords to world grid coords
                            int wx = cx * CHUNK_SIZE + lx;
                            int wy = cy * CHUNK_SIZE + ly;
                            activeWater.push_back({wx, wy});  // Store world grid coords
                        }
                    }
                }
            }
        }

        void step(Pixel::ChunkGrid& grid, Pixel::PixelAttributes& attrs, std::vector<graphics::Pixel>& renderPixels, float deltaTime) override {
            _accumulator += deltaTime;
            if (_accumulator < _stepInterval) return;
            _accumulator -= _stepInterval;

            // simulate movement logic
            for (auto [x, y] : activeWater) {
                auto id = grid.getPixel(x, y);
                if (id == Pixel::EMPTY) continue;

                // Check and move down
                if (grid.getPixel(x, y - 1) == Pixel::EMPTY) {
                    grid.movePixel(x, y, x, y - 1);
                    renderPixels[attrs.renderIndex[id]].position.y -= PIXEL_SIZE;  // Use PIXEL_SIZE for world units
                    continue;
                }
                // Check and move left
                if (grid.getPixel(x - 1, y) == Pixel::EMPTY) {
                    grid.movePixel(x, y, x - 1, y);
                    renderPixels[attrs.renderIndex[id]].position.x -= PIXEL_SIZE;
                    continue;
                }
                // Check and move right
                if (grid.getPixel(x + 1, y) == Pixel::EMPTY) {
                    grid.movePixel(x, y, x + 1, y);
                    renderPixels[attrs.renderIndex[id]].position.x += PIXEL_SIZE;
                }
            }
        }

    private:
        std::vector<std::pair<int,int>> activeWater;
        float _accumulator = 0.0f;
        const float _stepInterval = 0.5f; // seconds
    };
}