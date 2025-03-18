#include <projectEditor/ProjectEditor.hpp>

ProjectEditor::ProjectEditor(std::shared_ptr<graphic::Graphic> graphic,
                             std::shared_ptr<Core> core) : _graphic(graphic), _core(core)
{
}

ProjectEditor::~ProjectEditor()
{
}

int ProjectEditor::run()
{
    graphic::EventType event;

    _graphic->isSpriteEditor = false;
    _graphic->isProjectEditor = true;

    while (_graphic->_windowOpen)
    {
        event = _graphic->checkEvent();

        if (event == graphic::EventType::WINDOW_CLOSE)
            return 0;

        if (event == graphic::EventType::KEY_TAB)
        {
            _graphic->tabSelectorData.tabIndex += 1;
            return 0;
        }

        if (!_graphic->projectData.spritePath.empty())
        {
            try
            {
                graphic::Position position;
                if (event == graphic::EventType::KEY_ESCAPE)
                {
                    _graphic->projectData.showImportSprite = false;
                    // Need to add this line if we want to clear the sprite path after loading the sprite
                    _graphic->projectData.spritePath.clear();
                    // _graphic->projectData.oldSpritePath.clear();
                }
                if (event == graphic::EventType::MOUSE_CLICK_LEFT)
                {
                    position = _graphic->getPosition();
                    float zoom = static_cast<float>(_camera.zoom);
                    position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + _camera.position.x;
                    position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + _camera.position.y;

                    position.x = std::round(position.x);
                    position.y = std::round(position.y);
                    graphic::Sprite sprite = _graphic->loadSpriteFromJSON(_graphic->projectData.spritePath, false, position);
                    _core->addSprite(sprite);
                }
                _graphic->projectData.oldSpritePath.clear();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
        }

        moveCamera(event);
        zoomCamera(event);

        _graphic->clearWindow();

        _graphic->drawSprites(_core->getSprite(), _camera);
        _graphic->drawInterface(_camera);

        _graphic->updateWindow();
    }
    return 0;
}