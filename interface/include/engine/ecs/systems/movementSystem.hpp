#pragma once

#include <iostream>
#include <vector>

#include "engine/ecs/ISystem.hpp"
#include "engine/managers/componentManager.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/velocityComponent.hpp"

namespace ecs::systems
{
    class MovementSystem : public ISystem
    {
    public:
        void init() override {};

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