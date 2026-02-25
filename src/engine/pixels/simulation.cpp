#include <engine/pixels/simulation/simulation.hpp>

namespace Pixel
{
    void PixelSimulation::step(ChunkGrid& grid, PixelAttributes& attributes, std::vector<graphics::Pixel>& renderPixels, float deltaTime)
    {
        _elapsedTime += deltaTime;
        if (_elapsedTime < STEP_INTERVAL) return;
        _elapsedTime -= STEP_INTERVAL;

        _grid = &grid;
        _attributes = &attributes;
        _renderPixels = &renderPixels;

        simulateBottomUp();
        _pixelSimulated = !_pixelSimulated;

        _grid = nullptr;
        _attributes = nullptr;
        _renderPixels = nullptr;
    }

    void PixelSimulation::simulateBottomUp()
    {
        struct ChunkSnapshot {
            int cx, cy;
            PixelEntityID cells[CHUNK_SIZE][CHUNK_SIZE];
        };

        std::vector<ChunkSnapshot> snapshots;
        snapshots.reserve(_grid->getChunks().size());

        for (const auto& kv : _grid->getChunks()) {
            ChunkSnapshot snap;
            snap.cx = kv.first.first;
            snap.cy = kv.first.second;
            for (int ly = 0; ly < CHUNK_SIZE; ++ly)
                for (int lx = 0; lx < CHUNK_SIZE; ++lx)
                    snap.cells[lx][ly] = kv.second.get(lx, ly);
            snapshots.push_back(snap);
        }

        std::sort(snapshots.begin(), snapshots.end(),
                  [](const auto& a, const auto& b) {
                      return a.cy < b.cy;
                  });

        for (const auto& snap : snapshots) {
            for (int ly = 0; ly < CHUNK_SIZE; ++ly) {
                for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                    PixelEntityID id = snap.cells[lx][ly];
                    if (id == Pixel::EMPTY) continue;

                    if (_attributes->liquidAttributes.count(id) > 0) {
                        liquidSimulation(lx, ly, id, snap.cx, snap.cy);
                    }
                }
            }
        }
    }

    void PixelSimulation::liquidSimulation(int lx, int ly, PixelEntityID id, int cx, int cy)
    {
        Liquid& liquid = _attributes->liquidAttributes[id];

        // Skip if already processed this frame
        if (liquid.updateThisFrame == _pixelSimulated) return;
        liquid.updateThisFrame = !liquid.updateThisFrame;

        int wx = cx * CHUNK_SIZE + lx;
        int wy = cy * CHUNK_SIZE + ly;

        // Verify pixel is still at this position (may have moved already)
        if (_grid->getPixel(wx, wy) != id) return;

        // Try move down
        if (_grid->getPixel(wx, wy - 1) == Pixel::EMPTY) {
            _grid->movePixel(wx, wy, wx, wy - 1);
            (*_renderPixels)[_attributes->renderIndex[id]].position.y -= PIXEL_SIZE;
            return;
        }

        // Try spread left/right (random direction first)
        int dir = (rand() % 2 == 0) ? -1 : 1;
        if (_grid->getPixel(wx + dir, wy) == Pixel::EMPTY) {
            _grid->movePixel(wx, wy, wx + dir, wy);
            (*_renderPixels)[_attributes->renderIndex[id]].position.x += dir * PIXEL_SIZE;
            return;
        }
        if (_grid->getPixel(wx - dir, wy) == Pixel::EMPTY) {
            _grid->movePixel(wx, wy, wx - dir, wy);
            (*_renderPixels)[_attributes->renderIndex[id]].position.x -= dir * PIXEL_SIZE;
        }
    }
} // namespace Pixel