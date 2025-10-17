#include <editors/spriteEditor.hpp>

namespace editors {
    SpriteEditor::SpriteEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer)
        : _graphicsInterface(graphicsInterface), _renderer(renderer) {}

    SpriteEditor::~SpriteEditor() {}

    void SpriteEditor::run(graphics::InputEventType eventType) {
        handleEvents(eventType);

        _renderer->clear();
        _renderer->drawGrid(10.0f, 0.9f, 0.9f, 0.9f); // Draw grid with cell size 16
        _renderer->drawPixels(_spritePixels, 10.0f);
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
        std::cout << "Mouse left click detected in SpriteEditor." << std::endl;
    }
} // namespace editors