#pragma once

#include <vector>
#include "engine/ecs/entity.hpp"

namespace engine
{
    class EntityManager
    {
    public:
        EntityManager();
        ~EntityManager();

        ecs::Entity* createEntity();
        void destroyEntity(ecs::Entity* entity);
        const std::vector<ecs::Entity*>& getEntities() const;

    private:
        std::vector<ecs::Entity *> entities;
        std::vector<ecs::EntityID> availableIds;
        ecs::EntityID nextId = 1;
    };
} // namespace engine
