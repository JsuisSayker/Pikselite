#pragma once
#include "entity.hpp"

namespace ecs
{
    /**
     * @brief The IComponentArray interface defines the basic operations that a component array must implement.
     * This includes handling the destruction of entities and providing a virtual destructor for proper cleanup.
     */
    struct IComponentArray
    {
        virtual ~IComponentArray() = default;
        virtual void entityDestroyed(ecs::EntityID id) = 0;
    };
} // namespace ecs