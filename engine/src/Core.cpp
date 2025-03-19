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
    std::vector<graphic::Sprite> TmpSprites = this->_sprites;
    graphic::EventType event;
    std::shared_ptr<graphic::Graphic> graphic = std::make_shared<graphic::Graphic>(false);
    this->_systemManager->addSystem(std::make_unique<MovementSystem>());
    this->_systemManager->addSystem(std::make_unique<SandSystem>());
    this->_clock.restart();
    while (graphic->_windowOpen)
    {
        event = graphic->checkEvent();

        if (event == graphic::EventType::WINDOW_CLOSE)
            return 0;

        if (event != graphic::EventType::NONE)
            this->_systemManager->addEvent(event);

        this->_systemManager->updateSystems(this->_clock, TmpSprites);

        if (this->_systemManager->hasEvent())
            this->_systemManager->popEvent(event);

        graphic->clearWindow();

        graphic->drawSprites(TmpSprites, camera);
        graphic->drawFps(this->_clock);

        this->_clock.restart();
        graphic->updateWindow();
    }
    return 0;
}
