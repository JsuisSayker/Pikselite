#include <projectEditor/ProjectEditor.hpp>

void ProjectEditor::moveCamera(graphic::EventType event)
{
    if (event == graphic::EventType::KEY_ARROW_UP)
    {
        _camera.position.y -= 10;
    }
    if (event == graphic::EventType::KEY_ARROW_DOWN)
    {
        _camera.position.y += 10;
    }
    if (event == graphic::EventType::KEY_ARROW_LEFT)
    {
        _camera.position.x -= 10;
    }
    if (event == graphic::EventType::KEY_ARROW_RIGHT)
    {
        _camera.position.x += 10;
    }
}

void ProjectEditor::zoomCamera(graphic::EventType event)
{
    if (event == graphic::EventType::KEY_I)
    {
        _camera.zoom += 1;
    }
    if (event == graphic::EventType::KEY_O)
    {
        _camera.zoom -= 1;
        if (_camera.zoom < 4)
            _camera.zoom = 4;
    }
}
