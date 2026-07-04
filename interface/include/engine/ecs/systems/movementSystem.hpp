#pragma once

#include "engine/ecs/ISystem.hpp"
#include "engine/ecs/components/physicsComponent.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/velocityComponent.hpp"
#include "engine/managers/componentManager.hpp"

#include <iostream>
#include <tracy/Tracy.hpp>
#include <vector>

#ifndef TRACY_ENABLE
// output a warning if profiling is disabled
#pragma message(                                                                                   \
    "Tracy profiling is disabled. To enable, set PIKSELITE_ENABLE_PROFILING=ON in CMake and rebuild.")
#error "Not set"
#endif

namespace ecs::systems
{
    /**
     * @brief The MovementSystem is responsible for updating the position of entities based on their
     * velocity. It iterates through all entities that have both a Transform and Velocity component,
     * and updates their position according to the formula: new_position = old_position + velocity *
     * delta_time. This system is typically called once per frame during the game loop to ensure
     * smooth movement of entities across the screen.
     *
     */
    class MovementSystem : public ISystem
    {
      public:
        void init() override {};

        /**
         * @brief The update function is called every frame and is responsible for updating the
         * position of entities based on their velocity.
         *
         * @param dt
         * @param componentManager
         */
        void update(double dt, engine::ComponentManager& componentManager) override
        {
            ZoneScopedN("ECS::MovementSystem");
            for (auto entity : entities)
            {
                // Unity-like authority rule: if physics is active on an entity,
                // MovementSystem must not write Transform for that entity.
                if (componentManager.hasComponent<components::PhysicsBody>(entity))
                {
                    const auto& physics =
                        componentManager.getComponent<components::PhysicsBody>(entity);
                    if (physics.enabled)
                    {
                        continue;
                    }
                }

                auto& transform = componentManager.getComponent<components::Transform>(entity);
                auto& velocity = componentManager.getComponent<components::Velocity>(entity);

                transform.x += velocity.vx * dt;
                transform.y += velocity.vy * dt;
            }
        }
    };
} // namespace ecs::systems