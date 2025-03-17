#include <core/Core.hpp>

Core::Core()
{
}

Core::~Core()
{
}

void Core::addSprite(graphic::Sprite sprite)
{
    this->_Sprites.push_back(sprite);
}

int Core::run(graphic::Camera camera)
{
    graphic::EventType event;
    std::shared_ptr<graphic::Graphic> graphic = std::make_shared<graphic::Graphic>(false);
    float deltaTime = 0.0f;
    // add systems
    this->_systemManager->addSystem(std::make_unique<MovementSystem>());

    while (graphic->_windowOpen) {
        event = graphic->checkEvent();

        if (event == graphic::EventType::WINDOW_CLOSE)
            return 0;

        this->_systemManager->updateSystems(deltaTime);
        graphic->clearWindow();

        graphic->drawSprites(this->_Sprites, camera);

        graphic->updateWindow();
        
    }
    return 0;
}
