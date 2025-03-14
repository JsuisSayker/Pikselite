#include <spriteEditor/SpriteEditor.hpp>

void SpriteEditor::moveCamera(graphic::EventType event)
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

void SpriteEditor::zoomCamera(graphic::EventType event)
{
    if (event == graphic::EventType::KEY_I)
    {
        _camera.zoom += 1;
        if (_camera.zoom > 20)
            _camera.zoom = 20;
    }
    if (event == graphic::EventType::KEY_O)
    {
        _camera.zoom -= 1;
        if (_camera.zoom < 4)
            _camera.zoom = 4;
    }
}
