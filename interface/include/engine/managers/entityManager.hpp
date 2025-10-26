#pragma once

#include <vector>
#include <memory>
#include "engine/ecs/entity.hpp"

namespace engine
{
    class EntityManager
    {
    public:
        EntityManager();
        ~EntityManager();

        ecs::Entity createEntity();
        void destroyEntity(ecs::Entity entity);
        const std::vector<std::unique_ptr<ecs::Entity>>& getEntities() const;

    private:
        std::vector<std::unique_ptr<ecs::Entity>> entities;
        std::vector<ecs::EntityID> availableIds;
        ecs::EntityID nextId = 1;
    };
} // namespace engine