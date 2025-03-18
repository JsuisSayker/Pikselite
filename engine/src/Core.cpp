#include <core/Core.hpp>

Core::Core()
{
}

Core::~Core()
{
}

void Core::addSprite(graphic::Sprite sprite)
{
    this->_sprites.push_back(sprite);
}

int Core::run(graphic::Camera camera)
{
    graphic::EventType event;
    std::shared_ptr<graphic::Graphic> graphic = std::make_shared<graphic::Graphic>(false);
    this->_clock.restart();
    this->_systemManager->addSystem(std::make_unique<MovementSystem>());

    while (graphic->_windowOpen)
    {
        event = graphic->checkEvent();

        if (event == graphic::EventType::WINDOW_CLOSE)
            return 0;

        if (event != graphic::EventType::NONE)
            this->_systemManager->addEvent(event);

        this->_systemManager->updateSystems(this->_clock.getElapsedTime(), this->_sprites);

        if (this->_systemManager->hasEvent())
            this->_systemManager->popEvent(event);

        graphic->clearWindow();

        graphic->drawSprites(this->_sprites, camera);

        this->_clock.restart();
        graphic->updateWindow();
    }
    return 0;
}
