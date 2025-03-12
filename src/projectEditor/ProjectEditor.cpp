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

        moveCamera(event);
        zoomCamera(event);

        _graphic->clearWindow();

        _graphic->drawInterface();

        _graphic->updateWindow();
    }
    return 0;
}