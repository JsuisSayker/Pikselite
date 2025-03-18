#pragma once


#include <vector>
#include <memory>

#include "systems/ISystem.hpp"

#include <utils/EngineEnum.hpp>

class SystemManager
{
public:
    SystemManager() = default;
    ~SystemManager() = default;

    void addSystem(std::unique_ptr<ISystem> system);

    void addEvent(graphic::EventType event);
    void removeEvent(graphic::EventType event);
    void popEvent(graphic::EventType event);
    bool hasEvent();

    void updateSystems(float deltaTime, std::vector<graphic::Sprite> &sprites);

private:
    std::vector<std::unique_ptr<ISystem>> systems;
    std::vector<graphic::EventType> eventStack;
};
