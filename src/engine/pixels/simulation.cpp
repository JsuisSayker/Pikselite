#include <engine/pixels/simulation/simulation.hpp>

namespace Pixel
{
    void PixelSimulation::step(ChunkGrid& grid, PixelAttributes& attributes, std::vector<graphics::Pixel>& renderPixels, float deltaTime)
    {
        // Store references as pointers for use in other functions
        _grid = &grid;
        _attributes = &attributes;
        _renderPixels = &renderPixels;

        static float elapsed = 0.0f;
        elapsed += deltaTime;
        
        if (elapsed >= 1.0f) {
            simulateBottomUp(elapsed);
            elapsed = 0.0f;
        }

        // Clear pointers (optional, for safety)
        _grid = nullptr;
        _attributes = nullptr;
        _renderPixels = nullptr;
    }

    void PixelSimulation::simulateBottomUp(float deltaTime)
    {
        // Get all chunks
        const auto& chunks = _grid->getChunks();
        
        // Sort chunks by Y coordinate (bottom to top: ascending Y)
        std::vector<std::pair<std::pair<int,int>, Chunk*>> sortedChunks;
        for (auto& kv : chunks) {
            int cx = kv.first.first;
            int cy = kv.first.second;
            sortedChunks.push_back({{cx, cy}, const_cast<Chunk*>(&kv.second)});
        }
        std::sort(sortedChunks.begin(), sortedChunks.end(),
                  [](const auto& a, const auto& b) {
                      return a.first.second < b.first.second; // Sort by cy ascending (bottom to top)
                  });

        // Iterate chunks bottom to top
        for (const auto& chunkEntry : sortedChunks) {
            int cx = chunkEntry.first.first;
            int cy = chunkEntry.first.second;
            Chunk* chunk = chunkEntry.second;

            // Iterate pixels within chunk bottom to top (ly ascending)
            for (int ly = 0; ly < CHUNK_SIZE; ++ly) {
                for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                    PixelEntityID id = chunk->get(lx, ly);
                    if (id == Pixel::EMPTY) continue;

                    if (_attributes->liquidAttributes.count(id) > 0) {
                        liquidSimulation(deltaTime, lx, ly, id, cx, cy);
                    }
                }
            }
        }
    }

    void PixelSimulation::liquidSimulation(float deltaTime, int lx, int ly, PixelEntityID id, int cx, int cy)
    {
        // Convert local chunk coords to world grid coords
        int wx = cx * CHUNK_SIZE + lx;
        int wy = cy * CHUNK_SIZE + ly;

        // Simple liquid simulation: try to move down
        if (_grid->getPixel(wx, wy - 1) == Pixel::EMPTY) {
            _grid->movePixel(wx, wy, wx, wy - 1);
            (*_renderPixels)[_attributes->renderIndex[id]].position.y -= PIXEL_SIZE;
        } else {
            // // Try spread left/right
            // int direction = (rand() % 2 == 0) ? -1 : 1;
            // int nx = wx + direction;
            
            // if (_grid->getPixel(nx, wy) == Pixel::EMPTY) {
            //     _grid->movePixel(wx, wy, nx, wy);
            //     (*_renderPixels)[_attributes->renderIndex[id]].position.x += direction * PIXEL_SIZE;
            // }
        }
    }
} // namespace Pixel