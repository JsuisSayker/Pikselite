#pragma once
#include <cstdint>

namespace ecs
{
    using EntityID = std::uint32_t;
    constexpr EntityID MAX_ENTITIES = 5000;

    struct Entity
    {
        EntityID id;
        explicit Entity(EntityID id = 0) : id(id) {}
        bool operator==(const Entity &other) const { return id == other.id; }
        bool operator!=(const Entity &other) const { return id != other.id; }
    };
}
