#include "managers/SystemManager.hpp"

void SystemManager::addSystem(std::unique_ptr<ISystem> system)
{
    systems.push_back(std::move(system));
}

void SystemManager::updateSystems(float deltaTime)
{
    for (auto &system : systems)
    {
        system->update(deltaTime);
    }
}
