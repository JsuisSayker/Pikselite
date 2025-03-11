#include <editor/Editor.hpp>

void Editor::moveCamera(graphic::EventType event)
{
    if (event == graphic::EventType::KEY_ARROW_UP)
    {
        _graphic->camera.position.y -= 10;
    }
    if (event == graphic::EventType::KEY_ARROW_DOWN)
    {
        _graphic->camera.position.y += 10;
    }
    if (event == graphic::EventType::KEY_ARROW_LEFT)
    {
        _graphic->camera.position.x -= 10;
    }
    if (event == graphic::EventType::KEY_ARROW_RIGHT)
    {
        _graphic->camera.position.x += 10;
    }
}

void Editor::zoomCamera(graphic::EventType event)
{
    if (event == graphic::EventType::KEY_I)
    {
        _graphic->camera.zoom += 1;
    }
    if (event == graphic::EventType::KEY_O)
    {
        _graphic->camera.zoom -= 1;
        if (_graphic->camera.zoom < 4)
            _graphic->camera.zoom = 4;
    }
}
