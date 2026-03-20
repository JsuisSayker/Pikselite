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
#include <vector>
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

    /**
     * @brief Structs for managing pending sprite placement, including the cells that will be affected by the sprite, the original pixel entity IDs and attributes of those cells, and a preview of the pixels that will be placed. This allows for a preview mode when placing sprites, where the user can see which pixels will be affected and what the new pixels will look like before confirming the placement.
     */
    struct PendingCell {
        int localGX = 0;
        int localGY = 0;
        Pixel::PixelEntityID oldId = Pixel::EMPTY;
    };

    /**
     * @brief Struct for managing the state of a sprite that is pending placement in the scene.
     * valid: Indicates whether there is a valid pending sprite ready for placement.
     * cells: A vector of PendingCell structs representing the cells that will be affected by the sprite placement, including their local grid coordinates and original pixel entity IDs.
     * previewLocalPixels: A vector of graphics::Pixel structs representing the preview of the pixels that will be placed by the sprite, in local world units (based on PIXEL_SIZE).
     * oldRenderIndex: A mapping of PixelEntityIDs to their original render index in the _renderPixels vector, allowing for restoration of the original pixels if the sprite placement is canceled.
     * solids, liquids, gases: Mappings of PixelEntityIDs to their original solid, liquid, and gaseous attributes, allowing for restoration of the original pixel attributes if the sprite placement is canceled.
     * loadedRenderPixels: A vector of graphics::Pixel structs representing the pixels that will be
     * rendered for the sprite, loaded from the sprite file and transformed to world coordinates based on the anchor point during placement.
     */
    struct PendingSprite {
        bool valid = false;
        std::vector<PendingCell> cells;
        std::vector<graphics::Pixel> previewLocalPixels; // local world units (PIXEL_SIZE-based)
        std::unordered_map<Pixel::PixelEntityID, int> oldRenderIndex;
        std::unordered_map<Pixel::PixelEntityID, Pixel::Solid> solids;
        std::unordered_map<Pixel::PixelEntityID, Pixel::Liquid> liquids;
        std::unordered_map<Pixel::PixelEntityID, Pixel::Gaseous> gases;
        std::vector<graphics::Pixel> loadedRenderPixels;
    };
}
