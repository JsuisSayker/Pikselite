#pragma once
#include <engine/pixels/pixelEnum.hpp>
#include <array>

namespace Pixel
{
    static constexpr int CHUNK_SIZE = 32;
    struct PairHash {
        std::size_t operator()(const std::pair<int,int>& p) const noexcept {
            return (std::hash<int>()(p.first) * 73856093) ^
                   (std::hash<int>()(p.second) * 19349663);
        }
    };

    class Chunk
    {
        public:
            Chunk() {
                for (int x = 0; x < CHUNK_SIZE; ++x) {
                    for (int y = 0; y < CHUNK_SIZE; ++y) {
                        cells[x][y] = EMPTY;
                    }
                }
            }

            PixelEntityID get(int x, int y) const {
                for (int i = 0; i < CHUNK_SIZE; ++i) {
                    for (int j = 0; j < CHUNK_SIZE; ++j) {
                        if (x == i && y == j) {
                            return cells[x][y];
                        }
                    }
                }
                return EMPTY;
            }

            void set(int x, int y, PixelEntityID id) {
                if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE) {
                    return;
                }
                cells[x][y] = id;
            }

        private:
            PixelEntityID cells[CHUNK_SIZE][CHUNK_SIZE];
    };

    class ChunkGrid {
        using ChunkCoord = std::pair<int, int>;
        public:
            Chunk& getOrCreateChunk(int cx, int cy) {
                return chunks[{cx, cy}];
            }

            Chunk* getChunk(int cx, int cy) {
                auto it = chunks.find({cx, cy});
                return (it == chunks.end()) ? nullptr : &it->second;
            }

            const std::unordered_map<ChunkCoord, Chunk, PairHash>& getChunks() const {
                return chunks;
            }

            PixelEntityID getPixel(int x, int y) {
                int cx = floor((float)x / CHUNK_SIZE);
                int cy = floor((float)y / CHUNK_SIZE);
                int lx = (x % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
                int ly = (y % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;

                Chunk* c = getChunk(cx, cy);
                if (!c) return Pixel::EMPTY;

                return c->get(lx, ly);
            }

            bool removePixel(int x, int y) {
                int cx = floor((float)x / CHUNK_SIZE);
                int cy = floor((float)y / CHUNK_SIZE);
                int lx = (x % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
                int ly = (y % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;

                Chunk* c = getChunk(cx, cy);
                if (!c) return false;

                c->set(lx, ly, Pixel::EMPTY);
                return true;
            }

            bool movePixel(int oldX, int oldY, int newX, int newY) {
                PixelEntityID id = getPixel(oldX, oldY);
                if (id == Pixel::EMPTY) return false;

                removePixel(oldX, oldY);
                int cx = floor((float)newX / CHUNK_SIZE);
                int cy = floor((float)newY / CHUNK_SIZE);
                int lx = (newX % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
                int ly = (newY % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;

                Chunk& c = getOrCreateChunk(cx, cy);
                c.set(lx, ly, id);
                return true;
            }

        private:
            std::unordered_map<ChunkCoord, Chunk, PairHash> chunks;
    };
} // namespace Pixel