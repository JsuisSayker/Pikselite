#pragma once

#include <graphic/Graphic.hpp>

#include <managers/SystemManager.hpp>
#include <systems/MovementSystem.hpp>
#include <systems/FireSystem.hpp>
#include <systems/SandSystem.hpp>

#include <utils/Clock.hpp>

#include <memory>

// the core is the engine that save entity and manage the game loop

class Core
{
public:
    Core();
    ~Core();

    std::vector<graphic::Sprite> getSprite() { return _sprites; }

    void addSprite(graphic::Sprite sprite);
    int run(graphic::Camera camera);

    std::vector<graphic::Sprite> _sprites;

protected:
private:
    std::shared_ptr<SystemManager> _systemManager = std::make_shared<SystemManager>();
    Clock _clock;
};
