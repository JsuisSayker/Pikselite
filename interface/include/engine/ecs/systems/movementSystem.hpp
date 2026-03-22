#pragma once

#include <iostream>
#include <vector>

#include "engine/ecs/ISystem.hpp"
#include "engine/managers/componentManager.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/velocityComponent.hpp"

namespace ecs::systems
{
    /**
     * @brief The MovementSystem is responsible for updating the position of entities based on their velocity.
     * It iterates through all entities that have both a Transform and Velocity component, and updates their position according to the formula: new_position = old_position + velocity * delta_time.
     * This system is typically called once per frame during the game loop to ensure smooth movement of entities across the screen.
     * 
     */
    class MovementSystem : public ISystem
    {
    public:
        void init() override {};

        /**
         * @brief The update function is called every frame and is responsible for updating the position of entities based on their velocity.
         * 
         * @param dt 
         * @param componentManager 
         */
        void update(double dt, engine::ComponentManager& componentManager) override
        {
            for (auto entity : entities)
            {
                auto &transform = componentManager.getComponent<components::Transform>(entity);
                auto &velocity = componentManager.getComponent<components::Velocity>(entity);

                transform.x += velocity.vx * dt;
                transform.y += velocity.vy * dt;
            }
        }
    };
} // namespace ecs::systems