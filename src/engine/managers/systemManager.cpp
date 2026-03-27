#include "engine/managers/systemManager.hpp"

namespace engine
{
    void SystemManager::update(double deltaTime, engine::ComponentManager& componentManager)
    {
        for (const auto &system : systems)
        {
            system->update(deltaTime, componentManager);
        }
    }

    void SystemManager::entitySignatureChanged(ecs::EntityID entity, const ecs::Signature &entitySignature)
    {
        for (auto &[type, system] : systemsMap)
        {
            const auto sigIt = systemSignatures.find(type);
            if (sigIt == systemSignatures.end())
            {
                // No declared signature: keep entity out of this system.
                system->entities.erase(entity);
                continue;
            }

            const auto &sysSig = sigIt->second;

            if ((entitySignature & sysSig) == sysSig)
            {
                system->entities.insert(entity);
            }
            else
            {
                system->entities.erase(entity);
            }
        }
    }

    void SystemManager::entityDestroyed(ecs::EntityID entity)
    {
        for (const auto &system : systems)
        {
            system->entities.erase(entity);
        }
    }

} // namespace engine