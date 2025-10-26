#pragma once

#include <array>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

#include "entity.hpp"
#include "IComponentArray.hpp"

namespace ecs
{
    constexpr std::size_t MAX_COMPONENTS = ecs::MAX_ENTITIES;

    template <typename T>
    class ComponentArray final : public IComponentArray
    {
    public:
        ComponentArray() = default;
        ~ComponentArray() override = default;

        void insertData(ecs::EntityID id, const T& component)
        {
            auto it = entityToIndex.find(id);
            if (it != entityToIndex.end()) {
                components[it->second] = component;
                return;
            }
            if (size >= MAX_COMPONENTS) {
                throw std::runtime_error("ComponentArray full");
            }
            std::size_t newIndex = size;
            components[newIndex] = component;
            entityToIndex[id] = newIndex;
            indexToEntity[newIndex] = id;
            ++size;
        }

        void removeData(ecs::EntityID id)
        {
            auto it = entityToIndex.find(id);
            if (it == entityToIndex.end()) return;

            std::size_t indexOfRemoved = it->second;
            std::size_t indexOfLast = size - 1;

            if (indexOfRemoved != indexOfLast) {
                // déplace le dernier dans le trou
                components[indexOfRemoved] = components[indexOfLast];

                ecs::EntityID lastEntity = indexToEntity[indexOfLast];
                entityToIndex[lastEntity] = indexOfRemoved;
                indexToEntity[indexOfRemoved] = lastEntity;
            }

            entityToIndex.erase(id);
            indexToEntity.erase(indexOfLast);
            --size;
        }

        T& getData(ecs::EntityID id)
        {
            auto it = entityToIndex.find(id);
            if (it == entityToIndex.end()) {
                throw std::out_of_range("Component not found for entity");
            }
            return components[it->second];
        }

        bool has(ecs::EntityID id) const
        {
            return entityToIndex.find(id) != entityToIndex.end();
        }

        // Wrappers pratiques si tu passes encore ecs::Entity
        void insertData(ecs::Entity e, const T& c) { insertData(e.id, c); }
        void removeData(ecs::Entity e) { removeData(e.id); }
        T& getData(ecs::Entity e) { return getData(e.id); }

        // IComponentArray
        void entityDestroyed(ecs::EntityID id) override { removeData(id); }

    private:
        std::array<T, MAX_COMPONENTS> components{};
        std::unordered_map<ecs::EntityID, std::size_t> entityToIndex;
        std::unordered_map<std::size_t, ecs::EntityID> indexToEntity;
        std::size_t size = 0;
    };

} // namespace ecs