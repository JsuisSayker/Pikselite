#include <projectEditor/ProjectEditor.hpp>

ProjectEditor::ProjectEditor(std::shared_ptr<graphic::Graphic> graphic,
                             std::shared_ptr<Core> core) : _graphic(graphic), _core(core)
{
}

ProjectEditor::~ProjectEditor()
{
}

bool ProjectEditor::spriteIsClicked(graphic::Sprite sprite, graphic::Position position)
{
    for (graphic::Pixel pixel : sprite.pixels)
    {
        if (pixel.position.x == position.x && pixel.position.y == position.y)
            return true;
    }
    return false;
}

void ProjectEditor::checkIfSpriteIsSelected()
{
    graphic::Position position = _graphic->getPosition();
    float zoom = static_cast<float>(_camera.zoom);
    position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + _camera.position.x;
    position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + _camera.position.y;
    position.x = std::round(position.x);
    position.y = std::round(position.y);

    for (int i = 0; i < _core->_sprites.size(); i++)
    {
        if (spriteIsClicked(_core->_sprites[i], position))
        {
            _core->_sprites[i].isSelected = true;
            _graphic->selectedSpriteActions = _core->_sprites[i].actions;
            _graphic->projectData.showSpriteInputSidebar = true;
        }
    }
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

        if (_graphic->projectData.runGame)
        {
            _graphic->projectData.runGame = false;
            _core->run(_camera);
        }

        if (event == graphic::EventType::MOUSE_CLICK_LEFT)
        {
            checkIfSpriteIsSelected();
        }

        if (!_graphic->projectData.spritePath.empty())
        {
            try
            {
                graphic::Position position;
                if (event == graphic::EventType::KEY_ESCAPE)
                {
                    _graphic->projectData.showImportSprite = false;
                    _graphic->projectData.spritePath.clear();
                    _graphic->projectData.selectedFileName.clear();
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
        if (_graphic->projectData.showGrid)
            _graphic->drawGrid(_camera);
        _graphic->drawInterface(_camera);

        _graphic->updateWindow();
    }
    return 0;
}