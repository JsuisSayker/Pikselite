/**
 * @file pixelEnum.hpp
 * @brief Defines enums and structs related to pixel attributes and game objects.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and
 * interactions.
 *
 */
#pragma once

#include <any>
#include <cstdint>
#include <engine/pixels/simulation/pixel.hpp>
#include <graphics/graphicsEnum.hpp>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

/**
 * @brief Contains enums and structs for managing pixel attributes and game objects.
 */
namespace Pixel
{
    // Type aliases for IDs
    using GameObjectID = std::uint32_t;

    // Special constant values
    static constexpr GameObjectID NO_SPRITE = 0;

    /**
     * @brief Represents a game object that can consist of multiple pixel entities.
     */
    struct GameObject
    {
        bool isActive = true;

        GameObjectID                id;
        std::string                 name;
        std::vector<Element::Pixel> pixels;
        std::vector<Element::Vec2i> pixelLocalCoords;

        // list of any for components
        std::unordered_map<std::type_index, std::any> components;

        // get component of type T, returns nullptr if not found
        template <typename T> T* getComponent()
        {
            auto it = components.find(std::type_index(typeid(T)));
            if (it != components.end())
            {
                return std::any_cast<T>(&(it->second));
            }
            return nullptr;
        }

        // add or replace component of type T
        template <typename T> void addComponent(const T& component)
        {
            components[std::type_index(typeid(T))] = component;
        }

        // remove component of type T
        template <typename T> void removeComponent()
        {
            components.erase(std::type_index(typeid(T)));
        }

        // has component of type T
        template <typename T> bool hasComponent() const
        {
            return components.find(std::type_index(typeid(T))) != components.end();
        }
    };
} // namespace Pixel
