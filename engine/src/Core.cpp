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
    this->_clock.restart();
    this->_systemManager->addSystem(std::make_unique<MovementSystem>());

    while (graphic->_windowOpen)
    {
        event = graphic->checkEvent();

        if (event == graphic::EventType::WINDOW_CLOSE)
            return 0;

        if (event != graphic::EventType::NONE)
        {
            for (int i = 0; i != _Sprites.size(); i++)
            {
                if (_Sprites[i].actions.find(event) != _Sprites[i].actions.end())
                {
                    this->_systemManager->addEvent(_Sprites[i].actions[event]);
                }
            }
        }

        this->_systemManager->updateSystems(this->_clock.getElapsedTime(), this->_Sprites);
        graphic->clearWindow();

        graphic->drawSprites(this->_Sprites, camera);

        this->_clock.restart();
        graphic->updateWindow();
    }
    return 0;
}
