#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>

#include "engine/ecs/ISystem.hpp"
#include "engine/ecs/signature.hpp"

#include "engine/ecs/systems/movementSystem.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/velocityComponent.hpp"
#include "engine/managers/componentManager.hpp"

namespace engine
{
    /**
     * @brief The SystemManager class is responsible for managing all systems in the ECS architecture.
     * It allows adding systems, updating them each frame, and notifying them of changes to entity signatures or when entities are destroyed.
     */
    class SystemManager
    {
    public:
        /**
         * @brief Updates all systems by calling their update function with the given delta time and component manager.
         * 
         * @param deltaTime 
         * @param componentManager 
         */
        void update(double deltaTime, engine::ComponentManager& componentManager);

        /**
         * @brief Notifies systems that an entity's signature has changed, so they can update their list of relevant entities accordingly.
         * 
         * @param entity 
         * @param entitySignature 
         */
        void entitySignatureChanged(ecs::EntityID entity, const ecs::Signature &entitySignature);

        /**
         * @brief Notifies systems that an entity has been destroyed, so they can remove it from their lists if necessary.
         * 
         * @param entity 
         */
        void entityDestroyed(ecs::EntityID entity);


        /**
         * @brief Adds a new system of type T to the manager and returns a reference to it.
         * 
         * @tparam T 
         * @tparam Args 
         * @param args 
         * @return T& 
         */
        template <typename T, typename... Args>
        T &addSystem(Args &&...args)
        {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T &ref = *system;

            systemsMap[typeid(T)] = system.get();
            systems.push_back(std::move(system));
            return ref;
        }

        /**
         * @brief Sets the signature for a system of type T, which defines which components an entity must have to be processed by that system.
         * 
         * @tparam T 
         * @param signature 
         */
        template <typename T>
        void setSignature(const ecs::Signature &signature)
        {
            systemSignatures[typeid(T)] = signature;
        }

        /**
         * @brief Gets a pointer to a system of type T. Returns nullptr if the system does not exist.
         * 
         * @tparam T 
         * @return T* 
         */
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
