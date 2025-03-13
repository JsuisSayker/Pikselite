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

        // Needs to change the position value by the one in the getRealPixelPosition method
        graphic::Pixel actualPixel = graphic::Pixel{false, _graphic->editorData.defaultColor, position};
        getRealPixelPosition(&actualPixel);
        // Snap to grid (integer grid)
        std::cout << "Position x AFTER: " << actualPixel.position.x << std::endl;
        std::cout << "Position y AFTER: " << actualPixel.position.x << std::endl;
        position.x = std::round(position.x);
        position.y = std::round(position.y);

        addPixel(graphic::Pixel{false, _graphic->editorData.defaultColor, position});
    }
}

void SpriteEditor::getRealPixelPosition(graphic::Pixel *pixel)
{
    std::pair<float, float> windowOrigin = std::make_pair(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    std::cout << "Window origin: " << windowOrigin.first << ", " << windowOrigin.second << std::endl;
    pixel->position.x = pixel->position.x - windowOrigin.first;
    pixel->position.y = pixel->position.y - windowOrigin.second;

    pixel->position.x = std::round(pixel->position.x);
    pixel->position.y = std::round(pixel->position.y);

    //absolute value in case of negative 0 for the x axis
    if(pixel->position.x == 0 || pixel->position.y == 0)
        pixel->position.x = -pixel->position.x;

    std::cout << "Position x: " << pixel->position.x << std::endl;
    std::cout << "Position y: " << pixel->position.y << std::endl;
}

void SpriteEditor::checkInterfaceEvents(graphic::EventType event)
{
    if (event == graphic::EventType::KEY_C) {
        _graphic->editorData.showColorSelector = !_graphic->editorData.showColorSelector;
    }
}
