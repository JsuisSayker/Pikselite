#include <spriteEditor/SpriteEditor.hpp>

void SpriteEditor::addPixel(graphic::Pixel pixel)
{
    for (graphic::Pixel &p : _graphic->_pixels)
    {
        if (p.position.x == pixel.position.x && p.position.y == pixel.position.y)
        {
            p.color = pixel.color;
            return;
        }
    }
    _graphic->_pixels.push_back(pixel);
}

void SpriteEditor::checkMouseEvents(graphic::EventType event)
{
    graphic::Position position;

    if (event == graphic::EventType::MOUSE_CLICK_LEFT || event == graphic::EventType::MOUSE_DRAG_LEFT)
    {
        position = _graphic->getPosition();
        float zoom = static_cast<float>(_camera.zoom);
        position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + _camera.position.x;
        position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + _camera.position.y;

        // Snap to grid (integer grid)
        position.x = std::round(position.x);
        position.y = std::round(position.y);

        addPixel(graphic::Pixel{false, _graphic->editorData.defaultColor, position});
    }
}

void SpriteEditor::checkInterfaceEvents(graphic::EventType event)
{
    if (event == graphic::EventType::KEY_C)
    {
        _graphic->editorData.showColorSelector = !_graphic->editorData.showColorSelector;
    }
}
