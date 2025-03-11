#include <editor/Editor.hpp>

void Editor::checkMouseEvents(graphic::EventType event)
{
    graphic::Position position;

    if (event == graphic::EventType::MOUSE_CLICK_LEFT || event == graphic::EventType::MOUSE_DRAG_LEFT)
    {
        position = _graphic->getPosition();
        float zoom = static_cast<float>(_graphic->camera.zoom);
        position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + _graphic->camera.position.x;
        position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + _graphic->camera.position.y;

        // Snap to grid (integer grid)
        position.x = std::round(position.x);
        position.y = std::round(position.y);

        _graphic->_pixels.push_back(graphic::Pixel{position, {255, 0, 0, 255}});
    }
}

void Editor::checkInterfaceEvents(graphic::EventType event)
{
    if (event == graphic::EventType::KEY_C) {
        _graphic->showColorSelector = !_graphic->showColorSelector;
    }
}
