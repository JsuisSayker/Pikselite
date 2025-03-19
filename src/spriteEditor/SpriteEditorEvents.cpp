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
        if (_graphic->selectedPixel != nullptr)
        {
            _graphic->editorData.showPixelEditorSidebar = false;
            _graphic->selectedPixel = nullptr;
        }

        position = _graphic->getPosition();
        float zoom = static_cast<float>(_camera.zoom);
        position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + _camera.position.x;
        position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + _camera.position.y;

        // Snap to grid (integer grid)
        position.x = std::round(position.x);
        position.y = std::round(position.y);

        addPixel(graphic::Pixel{false, _graphic->editorData.defaultColor, position, _graphic->editorData.defaultAttributes, _graphic->editorData.liquidEnabled, _graphic->editorData.solidEnabled, _graphic->editorData.lightEnabled, _graphic->editorData.fireEnabled, _graphic->editorData.flammableEnabled, _graphic->editorData.sandEnabled});
    }

    if (event == graphic::EventType::MOUSE_CLICK_RIGHT)
    {
        float zoom = static_cast<float>(_camera.zoom);

        position = _graphic->getPosition();
        position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + _camera.position.x;
        position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + _camera.position.y;
        position.x = std::round(position.x);
        position.y = std::round(position.y);

        graphic::Pixel *selectedPixel = findPixelAt(position);

        if (selectedPixel != nullptr)
        {
            _graphic->selectedPixel = selectedPixel;
            _graphic->editorData.showPixelEditorSidebar = true;
            _graphic->pixelEditorSidebarInitialized = false;
        }
    }
}
