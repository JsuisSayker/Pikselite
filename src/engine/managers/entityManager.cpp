#include "engine/ecs/entity.hpp"
#include "engine/managers/entityManager.hpp"

namespace engine
{
    EntityManager::EntityManager() = default;

    EntityManager::~EntityManager()
    {
        for (auto entity : entities)
        {
            delete entity;
        }
        entities.clear();
    }

    ecs::Entity* EntityManager::createEntity()
    {
        ecs::EntityID id;
        if (!availableIds.empty())
        {
            id = availableIds.back();
            availableIds.pop_back();
        }
        else
        {
            id = nextId++;
        }

        ecs::Entity* entity = new ecs::Entity(id);
        entities.push_back(entity);
        return entity;
    }

    void EntityManager::destroyEntity(ecs::Entity* entity)
    {
        auto it = std::find(entities.begin(), entities.end(), entity);
        if (it != entities.end())
        {
            availableIds.push_back(entity->id);
            delete *it;
            entities.erase(it);
        }
    }

    const std::vector<ecs::Entity*>& EntityManager::getEntities() const
    {
        return entities;
    }
} // namespace engine