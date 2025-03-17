#ifndef SYSTEM_MANAGER_HPP
#define SYSTEM_MANAGER_HPP

#include <vector>
#include <memory>
#include "systems/ISystem.hpp"

class SystemManager
{
public:
    SystemManager() = default;
    ~SystemManager() = default;

    void addSystem(std::unique_ptr<ISystem> system);

    void updateSystems(float deltaTime);

private:
    std::vector<std::unique_ptr<ISystem>> systems;
};

#endif // SYSTEM_MANAGER_HPP
