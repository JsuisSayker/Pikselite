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
                std::cout << "[MovementSystem] Updating entity " << entity << "\n";
                auto &transform = componentManager.getComponent<components::Transform>(entity);
                auto &velocity = componentManager.getComponent<components::Velocity>(entity);

                std::cout << "[MovementSystem] Entity " << entity
                          << " at (" << transform.x << ", " << transform.y << ") "
                          << "with velocity (" << velocity.vx << ", " << velocity.vy << ")\n";

                transform.x += velocity.vx * dt;
                transform.y += velocity.vy * dt;

                std::cout << "[MovementSystem] Entity " << entity
                          << " moved to (" << transform.x << ", " << transform.y << ")\n";
            }
        }
    };
} // namespace ecs::systems