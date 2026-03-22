#pragma once

#include <engine/pixels/simulation/pixel.hpp>
#include <iostream>
#include <unordered_map>

constexpr int CHUNK_SIZE = 32;

struct Chunk {
    Element::Pixel pixels[CHUNK_SIZE * CHUNK_SIZE];

    Chunk() {
        for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i)
            pixels[i].type = Element::EMPTY;
    }

    inline Element::Pixel& get(int x, int y)
    {
        return pixels[y * CHUNK_SIZE + x];
    }

    inline void set(int x, int y, Element::Pixel p)
    {
        pixels[y * CHUNK_SIZE + x] = p;
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
        return (v >= 0) ? v / CHUNK_SIZE : (v - CHUNK_SIZE + 1) / CHUNK_SIZE;
    }

    inline int mod(int v)
    {
        return (v % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
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


    Element::Pixel getPixel(int x, int y)
    {
        int cx = floorDiv(x);
        int cy = floorDiv(y);

        int lx = mod(x);
        int ly = mod(y);

        Chunk* chunk = getChunkIfExists(cx, cy);
        if (!chunk) {
            return Element::Pixel{Element::EMPTY};
        }

        return chunk->get(lx, ly);
    }

    Element::Pixel& getPixelRef(int x, int y)
    {
        int cx = floorDiv(x);
        int cy = floorDiv(y);

        int lx = mod(x);
        int ly = mod(y);

        Chunk& chunk = getOrCreateChunk(cx, cy);
        return chunk.get(lx, ly);
    }

    inline void setPixel(int x, int y, Element::Pixel p)
    {
        int cx = floorDiv(x);
        int cy = floorDiv(y);

        int lx = mod(x);
        int ly = mod(y);

        Chunk& chunk = getOrCreateChunk(cx, cy);
        chunk.set(lx, ly, p);
    }
};