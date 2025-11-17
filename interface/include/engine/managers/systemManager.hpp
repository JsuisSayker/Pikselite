#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>

#include "engine/ecs/ISystem.hpp"
#include "engine/ecs/signature.hpp"

#include "engine/ecs/systems/movementSystem.hpp"
#include "engine/ecs/components/transformComponent.hpp"

namespace engine
{
    class SystemManager
    {
    public:
        void update(double deltaTime);

        void entitySignatureChanged(ecs::EntityID entity, const ecs::Signature &entitySignature);

        void entityDestroyed(ecs::EntityID entity);

        template <typename T, typename... Args>
        T &addSystem(Args &&...args)
        {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T &ref = *system;

            systemsMap[typeid(T)] = system.get();
            systems.push_back(std::move(system));
            return ref;
        }

        template <typename T>
        void setSignature(const ecs::Signature &signature)
        {
            systemSignatures[typeid(T)] = signature;
        }

        template <typename T>
        T *getSystem()
        {
            for (auto &system : systems)
            {
                if (auto ptr = dynamic_cast<T *>(system.get()))
                    return ptr;
            }
            return nullptr;
        }

    private:
        std::vector<std::unique_ptr<ecs::ISystem>> systems;
        std::unordered_map<std::type_index, ecs::ISystem *> systemsMap;
        std::unordered_map<std::type_index, ecs::Signature> systemSignatures;
    };
} // namespace engine
