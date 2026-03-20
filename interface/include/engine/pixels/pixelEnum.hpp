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
     * @brief Struct representing solid properties of a pixel entity. This struct is used for managing solid pixel entities in the simulation, allowing for easy storage and manipulation of their physical properties during updates and interactions.
     */
    struct Solid {};
    /**
     * @brief Struct representing liquid properties of a pixel entity, including viscosity and a flag to indicate if it should be updated in the current frame. This struct is used for managing liquid pixel entities in the simulation, allowing for easy storage and manipulation of their physical properties during updates and interactions.
     * viscosity: A float representing the viscosity of the liquid pixel entity, which can be used to determine how it flows and interacts with other entities in the simulation.
     * updateThisFrame: A boolean flag indicating whether the liquid pixel entity should be updated during the current simulation frame. This can be used to optimize updates by only processing entities that are active or have changed state.
     */
    struct Liquid {
        float viscosity;
        bool updateThisFrame = false;
    };
    /**
     * @brief Struct representing gaseous properties of a pixel entity, including density and a flag to indicate if it should be updated in the current frame. This struct is used for managing gaseous pixel entities in the simulation, allowing for easy storage and manipulation of their physical properties during updates and interactions.
     * density: A float representing the density of the gaseous pixel entity, which can be used to determine how it interacts with other entities and how it behaves in the simulation.
     * updateThisFrame: A boolean flag indicating whether the gaseous pixel entity should be updated during the current simulation frame. This can be used to optimize updates by only processing entities that are active or have changed state.
     */
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
