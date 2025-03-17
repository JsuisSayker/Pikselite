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

void SystemManager::addEvent(engine::Events event)
{
    systemsOrder.push_back(event);
}

void SystemManager::removeEvent(engine::Events event)
{
    for (auto it = systemsOrder.begin(); it != systemsOrder.end(); it++)
    {
        if (*it == event)
        {
            systemsOrder.erase(it);
            return;
        }
    }
}