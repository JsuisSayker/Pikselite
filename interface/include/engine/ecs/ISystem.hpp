#pragma once

#include <set>
#include "entity.hpp"
#include <engine/managers/componentManager.hpp>

namespace ecs
{

    /**
     * @brief The ISystem interface defines the basic structure and functionality that all systems in the ECS architecture must implement.
     * 
     */
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        /**
         * @brief Updates the system's state.
         * 
         * @param deltaTime The time elapsed since the last update.
         * @param componentManager The component manager for accessing components.
         */
        virtual void update(double deltaTime, engine::ComponentManager& componentManager) = 0;

        /**
         * @brief Initializes the system. This function is called once when the system is added to the SystemManager.
         */
        virtual void init() = 0;

        /**
         * @brief A set of entities that this system is interested in.
         */
        std::set<EntityID> entities;
    };
} // namespace ecs