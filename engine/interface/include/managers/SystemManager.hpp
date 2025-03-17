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

    void addEvent(engine::Events event);
    void removeEvent(engine::Events event);

    void updateSystems(float deltaTime, std::vector<graphic::Sprite> sprites);

private:
    std::vector<std::unique_ptr<ISystem>> systems;
    std::vector<engine::Events> systemsOrder;
};
