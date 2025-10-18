#include <editors/spriteEditor.hpp>
#include <algorithm>

namespace editors {
    SpriteEditor::SpriteEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer)
        : _graphicsInterface(graphicsInterface), _renderer(renderer) {}

    SpriteEditor::~SpriteEditor() {}

    void SpriteEditor::run(graphics::InputEventType eventType) {
        handleEvents(eventType);

        _renderer->clear();
        _renderer->drawPixelsWCamera(_spritePixels, _camera, PIXEL_SIZE);
        _renderer->drawGrid(PIXEL_SIZE * _camera.getZoom(), {0.7f, 0.7f, 0.7f}); // Draw grid with cell size 16
        _renderer->present(_graphicsInterface->getWindow());
    }

    void SpriteEditor::handleEvents(graphics::InputEventType eventType) {
        switch (eventType) {
        case graphics::MOUSE_LEFT_CLICK:
            mouseLeftClick();
            break;
        case graphics::KEY_W:
            _camera.move(glm::vec2(0.0f, -PIXEL_SIZE));
            break;
        case graphics::KEY_S:
            _camera.move(glm::vec2(0.0f, PIXEL_SIZE));
            break;
        case graphics::KEY_A:
            _camera.move(glm::vec2(PIXEL_SIZE, 0.0f));
            break;
        case graphics::KEY_D:
            _camera.move(glm::vec2(-PIXEL_SIZE, 0.0f));
            break;
        case graphics::KEY_I:
            _camera.zoomIn(1.1f);
            break;
        case graphics::KEY_O:
            _camera.zoomOut(1.1f);
            break;
        default:
            break;
        }
    }

    void SpriteEditor::mouseLeftClick() {
        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        addPixel(mousePos.x, mousePos.y, 1.0f, 0.0f, 0.0f); // Add red pixel
    }

    void SpriteEditor::addPixel(float mx, float my, float r, float g, float b) {
        int winW, winH;
        SDL_GetWindowSize(_graphicsInterface->getWindow(), &winW, &winH);

        // Convert mouse to world
        glm::vec2 ndc;
        ndc.x = ( (float)mx / winW ) * 2.0f - 1.0f;
        ndc.y = - ( (float)my / winH ) * 2.0f + 1.0f;  // invert Y

        glm::mat4 invVP = glm::inverse(_camera.getViewProjection(winW, winH));
        glm::vec4 world4 = invVP * glm::vec4(ndc.x, ndc.y, 0.0f, 1.0f);
        
        float gx = std::round(world4.x / PIXEL_SIZE) * PIXEL_SIZE;
        float gy = std::round(world4.y / PIXEL_SIZE) * PIXEL_SIZE;
        glm::vec2 worldPos(gx, gy);

        // remove any existing pixel at (x, y)
        _spritePixels.erase(std::remove_if(_spritePixels.begin(), _spritePixels.end(),
            [worldPos](const graphics::Pixel& pixel) {
                return pixel.position.x == worldPos.x && pixel.position.y == worldPos.y;
            }), _spritePixels.end());

        _spritePixels.push_back({worldPos, {r, g, b}});
    }

} // namespace editors