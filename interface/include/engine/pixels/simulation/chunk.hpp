#pragma once

#include <engine/pixels/simulation/pixel.hpp>
#include <iostream>
#include <unordered_map>

constexpr int CHUNK_SIZE = 32;

/**
 * @brief Fixed-size chunk of pixels.
 */
struct Chunk
{
    Element::Pixel pixels[CHUNK_SIZE * CHUNK_SIZE];

    /**
     * @brief Initializes all pixels as empty.
     */
    Chunk()
    {
        for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i)
            pixels[i].type = Element::EMPTY;
    }

    /**
     * @brief Returns a mutable pixel in local chunk coordinates.
     * @param x Local X in range [0, CHUNK_SIZE).
     * @param y Local Y in range [0, CHUNK_SIZE).
     * @return Reference to the pixel stored at `(x, y)`.
     */
    inline Element::Pixel& get(int x, int y)
    {
        return pixels[y * CHUNK_SIZE + x];
    }

    /**
     * @brief Writes a pixel in local chunk coordinates.
     * @param x Local X in range [0, CHUNK_SIZE).
     * @param y Local Y in range [0, CHUNK_SIZE).
     * @param p Pixel value to store.
     * @return void
     */
    inline void set(int x, int y, Element::Pixel p)
    {
        pixels[y * CHUNK_SIZE + x] = p;
    }
};

/**
 * @brief Sparse world grid split into chunks.
 */
struct ChunkGrid
{
    std::unordered_map<int64_t, Chunk> chunks;

    /**
     * @brief Builds a packed key from chunk coordinates.
     * @param cx Chunk X coordinate.
     * @param cy Chunk Y coordinate.
     * @return Packed 64-bit key used in `chunks`.
     */
    inline static int64_t makeKey(int cx, int cy)
    {
        return (static_cast<int64_t>(cx) << 32) | (static_cast<uint32_t>(cy));
    }

    /**
     * @brief Integer floor division by `CHUNK_SIZE`.
     * @param v Global coordinate.
     * @return Chunk index for `v`.
     */
    inline static int floorDiv(int v)
    {
        return (v >= 0) ? v / CHUNK_SIZE : (v - CHUNK_SIZE + 1) / CHUNK_SIZE;
    }

    /**
     * @brief Positive modulo by `CHUNK_SIZE`.
     * @param v Global coordinate.
     * @return Local index in range `[0, CHUNK_SIZE)`.
     */
    inline static int mod(int v)
    {
        return (v % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
    }

    /**
     * @brief Returns a chunk if it exists.
     * @param cx Chunk X coordinate.
     * @param cy Chunk Y coordinate.
     * @return Pointer to chunk or `nullptr` when missing.
     */
    Chunk* getChunkIfExists(int cx, int cy)
    {
        auto it = chunks.find(makeKey(cx, cy));
        if (it == chunks.end())
            return nullptr;
        return &it->second;
    }

    /**
     * @brief Returns an existing chunk or creates it.
     * @param cx Chunk X coordinate.
     * @param cy Chunk Y coordinate.
     * @return Reference to the chunk at `(cx, cy)`.
     */
    Chunk& getOrCreateChunk(int cx, int cy)
    {
        return chunks[makeKey(cx, cy)];
    }

    /**
     * @brief Reads a pixel in global coordinates.
     * @param x Global X coordinate.
     * @param y Global Y coordinate.
     * @return Pixel value, or `Element::EMPTY` if the chunk does not exist.
     */
    Element::Pixel getPixel(int x, int y)
    {
        int cx = floorDiv(x);
        int cy = floorDiv(y);

        int lx = mod(x);
        int ly = mod(y);

        Chunk* chunk = getChunkIfExists(cx, cy);
        if (!chunk)
        {
            return Element::Pixel{Element::EMPTY};
        }

        return chunk->get(lx, ly);
    }

    /**
     * @brief Returns a mutable pixel in global coordinates.
     * @param x Global X coordinate.
     * @param y Global Y coordinate.
     * @return Reference to pixel storage (creates chunk if needed).
     */
    Element::Pixel& getPixelRef(int x, int y)
    {
        int cx = floorDiv(x);
        int cy = floorDiv(y);

        int lx = mod(x);
        int ly = mod(y);

        Chunk& chunk = getOrCreateChunk(cx, cy);
        return chunk.get(lx, ly);
    }

    /**
     * @brief Writes a pixel in global coordinates.
     * @param x Global X coordinate.
     * @param y Global Y coordinate.
     * @param p Pixel value to write.
     * @return void
     */
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