#pragma once
#include "entity.hpp"

namespace ecs {
struct IComponentArray {
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(ecs::EntityID id) = 0;
};
} // namespace ecs