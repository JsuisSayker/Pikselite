#pragma once

#include <set>
#include "entity.hpp"
#include <engine/managers/componentManager.hpp>

namespace ecs
{

    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        virtual void update(double deltaTime, engine::ComponentManager& componentManager) = 0;
        virtual void init() = 0;

        std::set<EntityID> entities;
    };
} // namespace ecs