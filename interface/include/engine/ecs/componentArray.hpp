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

        /**
         * @brief Inserts a component for a given entity ID. If the entity already has a component of this type, it will be overwritten.
         * If the maximum number of components is exceeded, an exception is thrown.
         * 
         * @param id 
         * @param component 
         */
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

        /**
         * @brief Removes the component associated with the given entity ID.
         * If the entity does not have a component of this type, the function does nothing.
         * 
         * @param id 
         */
        void removeData(ecs::EntityID id)
        {
            auto it = entityToIndex.find(id);
            if (it == entityToIndex.end()) return;

            std::size_t indexOfRemoved = it->second;
            std::size_t indexOfLast = size - 1;

            if (indexOfRemoved != indexOfLast) {
                components[indexOfRemoved] = components[indexOfLast];

                ecs::EntityID lastEntity = indexToEntity[indexOfLast];
                entityToIndex[lastEntity] = indexOfRemoved;
                indexToEntity[indexOfRemoved] = lastEntity;
            }

            entityToIndex.erase(id);
            indexToEntity.erase(indexOfLast);
            --size;
        }

        /**
         * @brief Get the Data object
         * 
         * @param id 
         * @return T& 
         */
        T& getData(ecs::EntityID id)
        {
            auto it = entityToIndex.find(id);
            if (it == entityToIndex.end()) {
                throw std::out_of_range("Component not found for entity");
            }
            return components[it->second];
        }

        /**
         * @brief Checks if a component exists for the given entity ID.
         * 
         * @param id 
         * @return true 
         * @return false 
         */
        bool has(ecs::EntityID id) const
        {
            return entityToIndex.find(id) != entityToIndex.end();
        }

        /**
         * @brief Handles the destruction of an entity by removing its associated component, if it exists.
         * 
         * @param id 
         */
        void entityDestroyed(ecs::EntityID id) override { removeData(id); }

    private:
        // densely packed array of components
        std::array<T, MAX_COMPONENTS> components{};

        // maps from EntityID to index in the components array
        std::unordered_map<ecs::EntityID, std::size_t> entityToIndex;

        // maps from index in the components array to EntityID (used for efficient removal)
        std::unordered_map<std::size_t, ecs::EntityID> indexToEntity;

        // number of valid components in the array
        std::size_t size = 0;
    };

} // namespace ecs