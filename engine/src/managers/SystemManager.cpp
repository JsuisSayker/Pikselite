#include "managers/SystemManager.hpp"

void SystemManager::addSystem(std::unique_ptr<ISystem> system)
{
    systems.push_back(std::move(system));
}

void SystemManager::updateSystems(Clock clock, std::vector<graphic::Sprite> &sprites)
{
    for (auto &system : systems)
    {
        system->update(clock, sprites, eventStack);
    }
}

void SystemManager::addEvent(graphic::EventType event)
{
    eventStack.push_back(event);
}

void SystemManager::popEvent(graphic::EventType event)
{
    eventStack.pop_back();
}

void SystemManager::removeEvent(graphic::EventType event)
{
    for (auto it = eventStack.begin(); it != eventStack.end(); it++)
    {
        if (*it == event)
        {
            eventStack.erase(it);
            return;
        }
    }
}

bool SystemManager::hasEvent()
{
    return !eventStack.empty();
}

void SystemManager::restartSystemsTimers()
{
    for (auto &system : systems)
    {
        system->restartTimer();
    }
}