#include <editors/spriteEditor.hpp>
#include <algorithm>

namespace editors {
    SpriteEditor::SpriteEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer)
        : _graphicsInterface(graphicsInterface), _renderer(renderer) {}

    SpriteEditor::~SpriteEditor() {}

    void SpriteEditor::run(graphics::InputEventType eventType) {
        handleEvents(eventType);

        _renderer->clear();
        _renderer->drawPixels(_spritePixels, PIXEL_SIZE);
        _renderer->drawGrid(PIXEL_SIZE, 0.7f, 0.7f, 0.7f); // Draw grid with cell size 16
        _renderer->present(_graphicsInterface->getWindow());
    }

    void SpriteEditor::handleEvents(graphics::InputEventType eventType) {
        switch (eventType) {
        case graphics::MOUSE_LEFT_CLICK:
            mouseLeftClick();
            break;
        default:
            break;
        }
    }

    void SpriteEditor::mouseLeftClick() {
        graphics::Coord mousePos = _graphicsInterface->getMousePosition();
        addPixel(mousePos.x, mousePos.y, 1.0f, 0.0f, 0.0f); // Add red pixel
    }

    void SpriteEditor::addPixel(float x, float y, float r, float g, float b) {
        float px = std::round(x / PIXEL_SIZE) * PIXEL_SIZE;
        float py = std::round(y / PIXEL_SIZE) * PIXEL_SIZE;

        // remove any existing pixel at (x, y)
        _spritePixels.erase(std::remove_if(_spritePixels.begin(), _spritePixels.end(),
            [px, py](const graphics::Pixel& pixel) {
                return pixel.x == px && pixel.y == py;
            }), _spritePixels.end());

        _spritePixels.push_back({px, py, r, g, b});
    }

} // namespace editors