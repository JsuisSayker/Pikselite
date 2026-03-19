/**
 * @file pixelEnum.hpp
 * @brief Defines enums and structs related to pixel attributes and game objects.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and interactions.
 *
 */
#pragma once

#include <graphics/graphicsEnum.hpp>
#include <unordered_map>
#include <cstdint>
#include <string>

/**
 * @brief Contains enums and structs for managing pixel attributes and game objects.
 */
namespace Pixel {
    // Type aliases for IDs
    using PixelEntityID = std::uint32_t;
    using GameObjectID = std::uint32_t;

    // Special constant values
    static constexpr PixelEntityID EMPTY = 0;
    static constexpr GameObjectID NO_SPRITE = 0;

    /**
     * Solid: Does not move, blocks movement.
     * Liquid: Moves down if possible, otherwise spreads left/right. Has viscosity affecting flow speed.
     * Gas: Moves up if possible, otherwise spreads left/right. Has density affecting flow speed
     */
    struct Solid {};
    struct Liquid {
        float viscosity;
        bool updateThisFrame = false;
    };
    struct Gaseous {
        float density;
        bool updateThisFrame = false;
    };

    /**
     * @brief Contains attributes for each pixel entity, including rendering index and physical properties.
     */
    struct PixelAttributes {
        std::unordered_map<PixelEntityID, int> renderIndex;
        std::unordered_map<PixelEntityID, Solid> solidAttributes;
        std::unordered_map<PixelEntityID, Liquid> liquidAttributes;
        std::unordered_map<PixelEntityID, Gaseous> gaseousAttributes;
    };

    /**
     * @brief Represents a game object that can consist of multiple pixel entities.
     */
    struct GameObject {
        bool isActive = true;
        
        GameObjectID id;
        std::string name;
        std::vector<PixelEntityID> pixelEntities;
    };

    /**
     * @brief Default properties for a pixel entity, including color and physical attributes.
     */
    struct DefaultPixelProperties {
        glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
        bool isSolid = false;
        bool isLiquid = false;
        bool isGaseous = false;
        Solid solidAttributes;
        Liquid liquidAttributes{0.5f};
        Gaseous gaseousAttributes{0.5f};
    };
}
