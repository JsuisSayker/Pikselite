#pragma once

#include <engine/pixels/pixelEnum.hpp>
#include <engine/pixels/chunk.hpp>
#include <random>


namespace Pixel {
    struct IPixelSystem {
        virtual void collectPixels(Pixel::ChunkGrid& grid, Pixel::PixelAttributes& attrs, std::vector<graphics::Pixel>& renderPixels) = 0;
        virtual void step(Pixel::ChunkGrid& grid, Pixel::PixelAttributes& attrs, std::vector<graphics::Pixel>& renderPixels, float deltaTime) = 0;
        virtual ~IPixelSystem() = default;
    };

    class LiquidSystem : public IPixelSystem {
    public:
        LiquidSystem() : _rng(std::random_device{}()) {}
        LiquidSystem(const LiquidSystem&) = delete;

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

            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            // simulate movement logic
            for (auto [x, y] : activeWater) {
                auto id = grid.getPixel(x, y);
                if (id == Pixel::EMPTY) continue;

                // Get viscosity (0.0 = water, 1.0 = honey/lava)
                float viscosity = attrs.liquidAttributes[id].viscosity;
                
                // Probability to move this frame (higher viscosity = lower chance)
                float moveChance = 1.0f - (viscosity * 0.8f); // water=1.0, honey=0.2
                if (dist(_rng) > moveChance) continue;

                // Try moving down
                if (grid.getPixel(x, y - 1) == Pixel::EMPTY) {
                    grid.movePixel(x, y, x, y - 1);
                    renderPixels[attrs.renderIndex[id]].position.y -= PIXEL_SIZE;  // Use PIXEL_SIZE for world units
                    continue;
                }

                // Horizontal spread (limited by viscosity)
                int spreadRange = static_cast<int>(3.0f * (1.0f - viscosity) + 1.0f); // water=3, honey=1
                int direction = (dist(_rng) < 0.5f) ? -1 : 1; // random left/right

                for (int i = 1; i <= spreadRange; ++i) {
                    int nx = x + (direction * i);
                    if (grid.getPixel(nx, y) == Pixel::EMPTY) {
                        grid.movePixel(x, y, nx, y);
                        renderPixels[attrs.renderIndex[id]].position.x += direction * i * PIXEL_SIZE;
                        break;
                    }
                }
            }
        }

    private:
        std::vector<std::pair<int,int>> activeWater;
        float _accumulator = 0.0f;
        const float _stepInterval = 0.1f; // check every 0.1s (adjust per viscosity via moveChance)
        std::mt19937 _rng;
    };
}