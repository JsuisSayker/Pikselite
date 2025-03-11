#include <editor/Editor.hpp>

Editor::Editor(std::shared_ptr<graphic::Graphic> graphic) : _graphic(graphic)
{
}

Editor::~Editor()
{
}

int Editor::run()
{
    graphic::EventType event;

    while (_graphic->_windowOpen)
    {
        event = _graphic->checkEvent();

        if (event == graphic::EventType::WINDOW_CLOSE)
            return 0;

        checkMouseEvents(event);
        checkInterfaceEvents(event);
        moveCamera(event);
        zoomCamera(event);

        _graphic->clearWindow();

        _graphic->drawPixels();
        _graphic->drawGrid();
        _graphic->drawInterface();
        
        _graphic->updateWindow();
    }
    return 0;
}