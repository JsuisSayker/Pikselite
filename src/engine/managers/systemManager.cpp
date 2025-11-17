#include "engine/managers/systemManager.hpp"

namespace engine
{
    void SystemManager::update(double deltaTime)
    {
        for (const auto &system : systems)
        {
            system->update(deltaTime);
        }
    }

    void SystemManager::entitySignatureChanged(ecs::EntityID entity, const ecs::Signature &entitySignature)
    {
        for (auto &[type, system] : systemsMap)
        {
            const auto &sysSig = systemSignatures[type];

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