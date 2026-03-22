/**
 * @file chunk.hpp
 * @brief Defines the Chunk and ChunkGrid classes for managing pixel entities in a spatial grid.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and interactions.
 */
#pragma once

#include <engine/pixels/pixelEnum.hpp>
#include <array>

/**
 * @brief Contains enums and structs for managing pixel attributes and game objects.
 */
namespace Pixel
{
    // Size of each chunk in pixels
    static constexpr int CHUNK_SIZE = 32;
    // Hash function for pair of integers (used for chunk coordinates)
    struct PairHash {
        std::size_t operator()(const std::pair<int,int>& p) const noexcept {
            return (std::hash<int>()(p.first) * 73856093) ^
                   (std::hash<int>()(p.second) * 19349663);
        }
    };

    /**
     * @brief Represents a chunk of pixels, containing a 2D array of PixelEntityIDs.
     * Each chunk manages a fixed-size grid of pixels, allowing for efficient spatial partitioning and access.
     * Chunks are stored in a ChunkGrid, which maps chunk coordinates to Chunk instances.
     */
    class Chunk
    {
        public:
            // Initializes all pixels in the chunk to EMPTY
            Chunk() {
                for (int x = 0; x < CHUNK_SIZE; ++x) {
                    for (int y = 0; y < CHUNK_SIZE; ++y) {
                        cells[x][y] = EMPTY;
                    }
                }
            }

            /**
             * @brief Retrieves the PixelEntityID at the specified local coordinates within the chunk.
             * @param x The local x-coordinate (0 to CHUNK_SIZE-1) within the chunk.
             * @param y The local y-coordinate (0 to CHUNK_SIZE-1) within the chunk.
             * @return The PixelEntityID at the specified coordinates, or EMPTY if out of bounds
             */
            PixelEntityID get(int x, int y) const {
                if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE) {
                    return EMPTY;
                }
                return cells[x][y];
            }

            /**
             * @brief Sets the PixelEntityID at the specified local coordinates within the chunk.
             * @param x The local x-coordinate (0 to CHUNK_SIZE-1) within the chunk.
             * @param y The local y-coordinate (0 to CHUNK_SIZE-1) within the chunk.
             * @param id The PixelEntityID to set at the specified coordinates.
             */
            void set(int x, int y, PixelEntityID id) {
                if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE) {
                    return;
                }
                cells[x][y] = id;
            }

        private:
            // 2D array of PixelEntityIDs representing the pixels in this chunk
            PixelEntityID cells[CHUNK_SIZE][CHUNK_SIZE];
    };

    // Grid managing all chunks, allowing for retrieval and manipulation of pixels based on world coordinates
    class ChunkGrid {
        // Type alias for chunk coordinates (cx, cy)
        using ChunkCoord = std::pair<int, int>;
        public:
            // Retrieves the chunk at the specified coordinates, creating it if it doesn't exist
            Chunk& getOrCreateChunk(int cx, int cy) {
                return chunks[{cx, cy}];
            }

            /**
             * @brief Retrieves the chunk at the specified coordinates, returning nullptr if it doesn't exist.
             * @param cx The chunk x-coordinate.
             * @param cy The chunk y-coordinate.
             * @return A pointer to the Chunk at the specified coordinates, or nullptr if it doesn't exist.
             */
            Chunk* getChunk(int cx, int cy) {
                auto it = chunks.find({cx, cy});
                return (it == chunks.end()) ? nullptr : &it->second;
            }

            /**
            * @brief Retrieves a const reference to the map of all chunks in the grid.
            * @return A const reference to the unordered_map containing chunk coordinates and their corresponding Chunk instances.
            */
            const std::unordered_map<ChunkCoord, Chunk, PairHash>& getChunks() const {
                return chunks;
            }

            /**
            * @brief Retrieves the PixelEntityID at the specified world coordinates.
            * @param x The world x-coordinate of the pixel.
            * @param y The world y-coordinate of the pixel.
            * @return The PixelEntityID at the specified coordinates, or EMPTY if out of bounds or chunk doesn't exist.
            */
            PixelEntityID getPixel(int x, int y) {
                int cx = floor((float)x / CHUNK_SIZE);
                int cy = floor((float)y / CHUNK_SIZE);
                int lx = (x % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
                int ly = (y % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;

                Chunk* c = getChunk(cx, cy);
                if (!c) return Pixel::EMPTY;

                return c->get(lx, ly);
            }

            /**
            * @brief Sets the PixelEntityID at the specified world coordinates.
            * @param x The world x-coordinate of the pixel.
            * @param y The world y-coordinate of the pixel.
            * @param id The PixelEntityID to set at the specified coordinates.
            */
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

            /**
             * @brief Moves a pixel from one set of world coordinates to another, updating the corresponding chunks and pixel IDs.
             * @param oldX The current world x-coordinate of the pixel.
             * @param oldY The current world y-coordinate of the pixel.
             * @param newX The new world x-coordinate to move the pixel to.
             * @param newY The new world y-coordinate to move the pixel to.
             * @return True if the pixel was successfully moved, false if the original position was empty or the move failed.
             */
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
            // Map of chunk coordinates to their corresponding Chunk instances, allowing for efficient retrieval and management of pixel data in a spatial grid
            std::unordered_map<ChunkCoord, Chunk, PairHash> chunks;
    };
} // namespace Pixel