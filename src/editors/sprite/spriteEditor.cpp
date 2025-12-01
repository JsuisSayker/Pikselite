#include <editors/sprite/spriteEditor.hpp>
#include <algorithm>

namespace editors {

    SpriteEditor::SpriteEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface)
        : _graphicsInterface(graphicsInterface), _renderer(renderer), _imguiInterface(imguiInterface) {}

    SpriteEditor::~SpriteEditor() {}

    void SpriteEditor::run(graphics::InputEventType eventType) {
        handleEvents(eventType);

        _renderer->clear();
        _imguiInterface->startFrame();

        _renderer->drawPixelsWCamera(_spritePixels, _camera, PIXEL_SIZE);
        _renderer->drawGrid(_camera, PIXEL_SIZE, {0.7f, 0.7f, 0.7f}); // Draw grid with cell size 16

        imguiHandling();
        _imguiInterface->endFrame(_graphicsInterface->getWindow());

        _renderer->present(_graphicsInterface->getWindow());
    }

    void SpriteEditor::handleEvents(graphics::InputEventType eventType) {
        switch (eventType) {
        case graphics::MOUSE_LEFT_CLICK:
            mouseLeftClick();
            break;
        case graphics::MOUSE_LEFT_DRAG:
            mouseLeftDrag();
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

    void SpriteEditor::imguiHandling() {
        _imguiInterface->showImGuiDemo();
        if (_showPixelEditor) {
            if (_currentPixel) {
                _imguiInterface->pixelEditor(*_currentPixel, "Pixel Color");
            }
        }
    }

    void SpriteEditor::mouseLeftClick() {
        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        graphics::Pixel* pixel = getPixelAt(worldPos);
        if (pixel) {
            _currentPixel = pixel;
            _showPixelEditor = true;
            return;
        }
        addPixel(worldPos, 1.0f, 0.0f, 0.0f);
    }

    void SpriteEditor::mouseLeftDrag() {
        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        removePixelAt(worldPos);
        _chunkGrid.removePixel((int)worldPos.x, (int)worldPos.y);
        addPixel(worldPos, 1.0f, 0.0f, 0.0f);
    }

    glm::vec2 SpriteEditor::screenToWorld(glm::vec2 screenPos) {
        int winW, winH;
        SDL_GetWindowSize(_graphicsInterface->getWindow(), &winW, &winH);

        // Convert mouse to world
        glm::vec2 ndc;
        ndc.x = ( (float)screenPos.x / winW ) * 2.0f - 1.0f;
        ndc.y = - ( (float)screenPos.y / winH ) * 2.0f + 1.0f;

        glm::mat4 invVP = glm::inverse(_camera.getViewProjection(winW, winH));
        glm::vec4 world4 = invVP * glm::vec4(ndc.x, ndc.y, 0.0f, 1.0f);
        
        float gx = std::round(world4.x / PIXEL_SIZE) * PIXEL_SIZE;
        float gy = std::round(world4.y / PIXEL_SIZE) * PIXEL_SIZE;

        return glm::vec2(gx, gy);
    }

    graphics::Pixel* SpriteEditor::getPixelAt(glm::vec2 worldPos) {
        for (auto& pixel : _spritePixels) {
            if (pixel.position == worldPos) {
                return &pixel;
            }
        }
        return nullptr;
    }

    bool SpriteEditor::removePixelAt(glm::vec2 worldPos) {
        auto it = std::remove_if(_spritePixels.begin(), _spritePixels.end(),
                                 [&worldPos](const graphics::Pixel& pixel) {
                                     return pixel.position == worldPos;
                                 });
        if (it != _spritePixels.end()) {
            _spritePixels.erase(it, _spritePixels.end());
            _chunkGrid.removePixel((int)worldPos.x, (int)worldPos.y);
            return true;
        }
        return false;
    }

    void SpriteEditor::addPixel(glm::vec2 worldPos, float r, float g, float b) {
        _spritePixels.push_back({worldPos, {r, g, b}});
        int cx = (int)std::floor(worldPos.x / Pixel::CHUNK_SIZE);
        int cy = (int)std::floor(worldPos.y / Pixel::CHUNK_SIZE);
        Pixel::Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);
        
        // Safe modulo for negative coordinates
        int lx = ((int)worldPos.x % Pixel::CHUNK_SIZE + Pixel::CHUNK_SIZE) % Pixel::CHUNK_SIZE;
        int ly = ((int)worldPos.y % Pixel::CHUNK_SIZE + Pixel::CHUNK_SIZE) % Pixel::CHUNK_SIZE;
        
        chunk.set(lx, ly, pixelIdCounter);
        pixelIdCounter++;
    }
} // namespace editors