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

graphic::Pixel *SpriteEditor::findPixelAt(const graphic::Position &pos)
{
    for (graphic::Pixel &p : _graphic->_pixels)
    {
        if (p.position.x == pos.x && p.position.y == pos.y)
        {
            return &p;
        }
    }
    return nullptr;
}

void SpriteEditor::checkMouseEvents(graphic::EventType event)
{
    graphic::Position position;

    if (event == graphic::EventType::MOUSE_CLICK_LEFT || event == graphic::EventType::MOUSE_DRAG_LEFT)
    {
        std::cout << "Mouse Click Left" << std::endl; ////////////////////
        position = _graphic->getPosition();
        float zoom = static_cast<float>(_camera.zoom);
        position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + _camera.position.x;
        position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + _camera.position.y;

        // Snap to grid (integer grid)
        position.x = std::round(position.x);
        position.y = std::round(position.y);

        addPixel(graphic::Pixel{false, _graphic->editorData.defaultColor, position, _graphic->editorData.defaultAttributes});
    }

    if (event == graphic::EventType::MOUSE_CLICK_RIGHT || event == graphic::EventType::MOUSE_DRAG_RIGHT)
    {
        std::cout << "Mouse Click Right" << std::endl; ////////////////////

        position = _graphic->getPosition();

        float zoom = static_cast<float>(_camera.zoom);

        position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + _camera.position.x;
        position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + _camera.position.y;

        position.x = std::round(position.x);
        position.y = std::round(position.y);

        graphic::Pixel *selectedPixel = findPixelAt(position);

        if (selectedPixel != nullptr)
        {
            std::cout << "Selected Pixel at: " << position.x << ", " << position.y << std::endl; ////////////////////
        } else {
            std::cout << "No pixel at: " << position.x << ", " << position.y << std::endl; ////////////////////
        }
    }
}
