#pragma once

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <iostream>

#include "engine/ecs/IComponentArray.hpp"
#include "engine/ecs/componentArray.hpp"

namespace engine
{
    using ComponentType = std::uint8_t;
    constexpr ComponentType MAX_COMPONENTS = 32;

    class ComponentManager
    {
    public:
        ComponentManager() = default;
        ~ComponentManager() = default;

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
            componentArrays.emplace(key, std::make_unique<ecs::ComponentArray<T>>());
        }

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

        template <typename T>
        void addComponent(ecs::Entity entity, const T &component)
        {
            getComponentArray<T>()->insertData(entity.id, component);
        }

        template <typename T>
        void removeComponent(ecs::Entity entity)
        {
            getComponentArray<T>()->removeData(entity.id);
        }

        template <typename T>
        T &getComponent(ecs::Entity entity)
        {
            return getComponentArray<T>()->getData(entity.id);
        }

        void entityDestroyed(ecs::Entity entity)
        {
            for (auto &pair : componentArrays)
            {
                pair.second->entityDestroyed(entity.id);
            }
        }

    private:
        std::unordered_map<std::type_index, ComponentType> componentTypes;
        std::unordered_map<std::type_index, std::unique_ptr<ecs::IComponentArray>> componentArrays;
        ComponentType nextComponentType = 0;

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