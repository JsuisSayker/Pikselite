#pragma once

#include <vector>
#include <memory>

#include "engine/ecs/entity.hpp"
#include "engine/ecs/signature.hpp"

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
        void setSignature(ecs::EntityID entity, const ecs::Signature &signature);
        ecs::Signature getSignature(ecs::EntityID entity);

    private:
        std::vector<std::unique_ptr<ecs::Entity>> entities;
        std::vector<ecs::EntityID> availableIds;
        std::unordered_map<ecs::EntityID, ecs::Signature> signatures;
        ecs::EntityID nextId = 1;
    };
} // namespace engine