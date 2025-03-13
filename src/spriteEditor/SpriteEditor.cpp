#include <spriteEditor/SpriteEditor.hpp>

SpriteEditor::SpriteEditor(std::shared_ptr<graphic::Graphic> graphic) : _graphic(graphic)
{
}

SpriteEditor::~SpriteEditor()
{
}

int SpriteEditor::run()
{
    graphic::EventType event;

    _graphic->isSpriteEditor = true;
    _graphic->isProjectEditor = false;

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

        if (!_graphic->projectData.folderPath.empty())
        {
            try
            {
                _graphic->saveSprite(_graphic->_pixels, _graphic->projectData.folderPath);
                _graphic->createExternalAttributeFile(_graphic->projectData.folderPath, _graphic->_pixels);
                _graphic->projectData.folderPath.clear();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
        }
        checkMouseEvents(event);
        checkInterfaceEvents(event);
        moveCamera(event);
        zoomCamera(event);

        _graphic->clearWindow();

        _graphic->drawPixels(_camera);
        if (_graphic->editorData.showGrid)
            _graphic->drawGrid(_camera);
        _graphic->drawInterface();

        _graphic->updateWindow();
    }
    return 0;
}