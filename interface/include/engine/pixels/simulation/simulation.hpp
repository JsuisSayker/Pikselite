/**
 * @file simulation.hpp
 * @brief Defines the PixelSimulation class responsible for simulating pixel interactions and physics.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and interactions.
 *
 */
#pragma once

#include <engine/pixels/pixelEnum.hpp>
#include <engine/pixels/chunk.hpp>

#include <algorithm>
#include <iostream>
#include <unordered_set>

/**
 * @brief Contains enums and structs for managing pixel attributes and game objects.
 */
namespace Pixel
{
    // Forward declaration of PixelSimulation class
    class PixelSimulation {
        public:
            // Constructor and destructor
            PixelSimulation() = default;
            ~PixelSimulation() = default;

            /**
             * @brief Advances the simulation by one step, updating pixel positions and interactions based on their attributes.
             * @param grid The ChunkGrid representing the current state of the pixel world.
             * @param attributes The PixelAttributes containing the properties of each pixel entity.
             * @param renderPixels The vector of pixels to be rendered.
             * @param deltaTime The time elapsed since the last simulation step.
             */
            void step(ChunkGrid& grid, PixelAttributes& attributes, std::vector<graphics::Pixel>& renderPixels, float deltaTime);
    
        private:
            // Pointers to the current grid, attributes, and render pixels for use during simulation steps
            ChunkGrid* _grid = nullptr;
            PixelAttributes* _attributes = nullptr;
            std::vector<graphics::Pixel>* _renderPixels = nullptr;

            float _elapsedTime = 0.0f;
            static constexpr float STEP_INTERVAL = 0.01f;
            bool _pixelSimulated = true;

            // Simulation methods for different pixel types and update orders
            void simulateBottomUp();
            void simulateTopDown();

            /**
             * @brief Simulates the behavior of liquid pixels, including movement and interactions with other pixels.
             * @param lx The local x-coordinate of the pixel within its chunk.
             * @param ly The local y-coordinate of the pixel within its chunk.
             * @param id The PixelEntityID of the pixel being simulated.
             * @param cx The chunk x-coordinate of the pixel.
             * @param cy The chunk y-coordinate of the pixel.
             */
            void liquidSimulation(int lx, int ly, PixelEntityID id, int cx, int cy);
            /**
             * @brief Simulates the behavior of gaseous pixels, including movement and interactions with other pixels.
             * @param lx The local x-coordinate of the pixel within its chunk.
             * @param ly The local y-coordinate of the pixel within its chunk.
             * @param id The PixelEntityID of the pixel being simulated.
             * @param cx The chunk x-coordinate of the pixel.
             * @param cy The chunk y-coordinate of the pixel.
             */
            void gasSimulation(int lx, int ly, PixelEntityID id, int cx, int cy);

            /**
            * @brief Encodes a pair of x and y coordinates into a single 64-bit integer for use in unordered sets.
            * @param x The x-coordinate to encode.
            * @param y The y-coordinate to encode.
            * @return An int64_t representing the encoded coordinates.
            */
            int64_t encodePos(int x, int y) const {
                return ((int64_t)(uint32_t)x << 32) | (uint32_t)y;
            }
    };
} // namespace Pixel