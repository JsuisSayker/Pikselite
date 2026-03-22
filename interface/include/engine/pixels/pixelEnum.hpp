/**
 * @file pixelEnum.hpp
 * @brief Defines enums and structs related to pixel attributes and game objects.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and interactions.
 *
 */
#pragma once

#include <graphics/graphicsEnum.hpp>
#include <engine/pixels/simulation/pixel.hpp>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

/**
 * @brief Contains enums and structs for managing pixel attributes and game objects.
 */
namespace Pixel {
    // Type aliases for IDs
    using GameObjectID = std::uint32_t;

    // Special constant values
    static constexpr GameObjectID NO_SPRITE = 0;

    /**
     * @brief Represents a game object that can consist of multiple pixel entities.
     */
    struct GameObject {
        bool isActive = true;

        GameObjectID id;
        std::string name;
        std::vector<Element::Pixel> pixels;
    };
}
