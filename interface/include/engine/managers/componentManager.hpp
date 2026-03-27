#pragma once

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <iostream>
#include <functional>

#include "engine/ecs/IComponentArray.hpp"
#include "engine/ecs/componentArray.hpp"

namespace engine
{
    using ComponentType = std::uint8_t;
    constexpr ComponentType MAX_COMPONENTS = 32;

    /**
     * @brief The ComponentManager class is responsible for managing the storage and access of components in the ECS architecture.
     * 
     */
    class ComponentManager
    {
    public:
        ComponentManager() = default;
        ~ComponentManager() = default;

        /**
         * @brief Create a Entity object
         * 
         * @return ecs::Entity 
         */
        void setEntityMutationCallback(std::function<void(ecs::EntityID)> callback)
        {
            onEntityMutated = std::move(callback);
        }

        /**
         * @brief Registers a component type with the manager.
         * 
         * @tparam T 
         */
        template <typename T>
        void registerComponent()
        {
            const std::type_index key = std::type_index(typeid(T));
            if (componentTypes.find(key) != componentTypes.end())
            {
                std::cerr << "Registering component type more than once: " << key.name() << std::endl;
                return;
            }

            componentTypes.emplace(key, nextComponentType++);
            auto array = std::make_unique<ecs::ComponentArray<T>>();
            componentArrays.emplace(key, std::move(array));
        }

        /**
         * @brief Gets the type of a component.
         * 
         * @tparam T 
         * @return ComponentType 
         */
        template <typename T>
        ComponentType getComponentType()
        {
            const std::type_index key = std::type_index(typeid(T));
            auto it = componentTypes.find(key);
            if (it == componentTypes.end())
            {
                std::cerr << "Component not registered before use: " << key.name() << std::endl;
                throw std::runtime_error("Component not registered before use");
            }
            return it->second;
        }

        /**
         * @brief Adds a component to an entity.
         * 
         * @tparam T 
         * @param entity 
         * @param component 
         */
        template <typename T>
        void addComponent(ecs::EntityID entity, const T &component)
        {
            getComponentArray<T>()->insertData(entity, component);
            notifyEntityMutated(entity);
        }

        /**
         * @brief Removes a component from an entity.
         * 
         * @tparam T 
         * @param entity 
         */
        template <typename T>
        void removeComponent(ecs::EntityID entity)
        {
            getComponentArray<T>()->removeData(entity);
            notifyEntityMutated(entity);
        }

        /**
         * @brief Gets a reference to a component of an entity.
         * 
         * @tparam T 
         * @param entity 
         * @return T& 
         */
        template <typename T>
        T &getComponent(ecs::EntityID entity)
        {
            return getComponentArray<T>()->getData(entity);
        }

        /**
         * @brief Checks if an entity has a component of a specific type.
         * 
         * @tparam T 
         * @param entity 
         * @return true 
         * @return false 
         */
        template <typename T>
        bool hasComponent(ecs::EntityID entity)
        {
            return getComponentArray<T>()->has(entity);
        }

        /**
         * @brief Notifies the component manager that an entity has been destroyed, so it can remove any associated components.
         * 
         * @param entity 
         */
        void entityDestroyed(ecs::EntityID entity)
        {
            for (auto &pair : componentArrays)
            {
                pair.second->entityDestroyed(entity);
            }
        }

    private:
        // Map of component type to component array
        std::unordered_map<std::type_index, ComponentType> componentTypes;

        // Map of component type to component array instance
        std::unordered_map<std::type_index, std::unique_ptr<ecs::IComponentArray>> componentArrays;

        // Incremental component type ID generator
        ComponentType nextComponentType = 0;
        std::function<void(ecs::EntityID)> onEntityMutated;

        void notifyEntityMutated(ecs::EntityID entity)
        {
            if (onEntityMutated)
            {
                onEntityMutated(entity);
            }
        }


        /**
         * @brief Gets the component array for a specific component type.
         * 
         * @tparam T 
         * @return ecs::ComponentArray<T>* 
         */
        template <typename T>
        ecs::ComponentArray<T> *getComponentArray()
        {
            const std::type_index key = std::type_index(typeid(T));

            if (componentTypes.find(key) == componentTypes.end())
            {
                std::cerr << "Component not registered before use: " << key.name() << std::endl;
                throw std::runtime_error("Component not registered before use");
            }
            return static_cast<ecs::ComponentArray<T> *>(componentArrays.at(key).get());
        }
    };
} // namespace engine