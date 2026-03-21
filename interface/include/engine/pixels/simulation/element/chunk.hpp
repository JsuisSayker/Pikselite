#pragma once

#include <engine/pixels/simulation/element/pixel.hpp>
#include <iostream>
#include <unordered_map>

constexpr int CHUNKS_SIZE = 32;

struct Chunk {
    simulation::Pixel pixels[CHUNKS_SIZE * CHUNKS_SIZE];

    Chunk() {
        for (int i = 0; i < CHUNKS_SIZE * CHUNKS_SIZE; ++i)
            pixels[i].type = simulation::EMPTY;
    }

    inline simulation::Pixel& get(int x, int y)
    {
        return pixels[y * CHUNKS_SIZE + x];
    }

    inline void set(int x, int y, simulation::Pixel p)
    {
        pixels[y * CHUNKS_SIZE + x] = p;
    }
};


struct ChunkGrid {
    std::unordered_map<int64_t, Chunk> chunks;

    inline int64_t makeKey(int cx, int cy)
    {
        return (static_cast<int64_t>(cx) << 32) | (static_cast<uint32_t>(cy));
    }

    inline int floorDiv(int v)
    {
        return (v >= 0) ? v / CHUNKS_SIZE : (v - CHUNKS_SIZE + 1) / CHUNKS_SIZE;
    }

    inline int mod(int v)
    {
        return (v % CHUNKS_SIZE + CHUNKS_SIZE) % CHUNKS_SIZE;
    }

    Chunk* getChunkIfExists(int cx, int cy)
    {
        auto it = chunks.find(makeKey(cx, cy));
        if (it == chunks.end()) return nullptr;
        return &it->second;
    }

    Chunk& getOrCreateChunk(int cx, int cy)
    {
        return chunks[makeKey(cx, cy)];
    }


    simulation::Pixel getPixel(int x, int y)
    {
        int cx = floorDiv(x);
        int cy = floorDiv(y);

        int lx = mod(x);
        int ly = mod(y);

        Chunk* chunk = getChunkIfExists(cx, cy);
        if (!chunk) {
            return simulation::Pixel{simulation::EMPTY};
        }

        return chunk->get(lx, ly);
    }

    simulation::Pixel& getPixelRef(int x, int y)
    {
        int cx = floorDiv(x);
        int cy = floorDiv(y);

        int lx = mod(x);
        int ly = mod(y);

        Chunk& chunk = getOrCreateChunk(cx, cy);
        return chunk.get(lx, ly);
    }

    inline void setPixel(int x, int y, simulation::Pixel p)
    {
        int cx = floorDiv(x);
        int cy = floorDiv(y);

        int lx = mod(x);
        int ly = mod(y);

        Chunk& chunk = getOrCreateChunk(cx, cy);
        chunk.set(lx, ly, p);
    }
};