#pragma once

#include "engine/ecs/ISystem.hpp"

#include <iostream>
#include <vector>

namespace ecs::systems
{
    class MovementSystem : public ISystem
    {
    public:
        void init() override{};

        void update(double dt) override
        {
            std::cout << "[MovementSystem] Update dt=" << dt << "\n";
        }
    };
} // namespace ecs::systems