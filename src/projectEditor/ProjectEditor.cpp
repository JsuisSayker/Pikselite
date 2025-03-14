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
                graphic::Sprite sprite = _graphic->loadSpriteFromJSON(_graphic->projectData.spritePath);
                _core->addSprite(sprite);
                _graphic->projectData.spritePath.clear();
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
        _graphic->drawInterface();

        _graphic->updateWindow();
    }
    return 0;
}