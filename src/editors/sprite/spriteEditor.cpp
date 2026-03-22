#include <editors/sprite/spriteEditor.hpp>
#include <algorithm>

namespace editors {

    SpriteEditor::SpriteEditor(
        graphics::Interface* graphicsInterface,
        graphics::Renderer* renderer,
        graphics::ImguiInterface* imguiInterface)
        : _graphicsInterface(graphicsInterface),
          _renderer(renderer),
          _imguiInterface(imguiInterface) {
    }

    SpriteEditor::~SpriteEditor() {
    }

    void SpriteEditor::run(const graphics::InputEvent& event) {
        handleEvents(event);

        _renderer->clear();
        _imguiInterface->startFrame();

        std::vector<graphics::Pixel> framePixels = buildRenderPixels();

        if (_isPlacingSprite && _pendingSprite.valid) {
            glm::vec2 mousePos = _graphicsInterface->getMousePosition();
            glm::vec2 worldPos = screenToWorld(mousePos);

            for (const auto& p : _pendingSprite.previewLocalPixels) {
                graphics::Pixel ghost = p;
                ghost.position.x += worldPos.x;
                ghost.position.y += worldPos.y;
                ghost.color *= 0.65f;
                framePixels.push_back(ghost);
            }
        }

        _renderer->drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);
        _renderer->drawGrid(_camera, PIXEL_SIZE, {0.7f, 0.7f, 0.7f});

        imguiHandling();
        _imguiInterface->endFrame(_graphicsInterface->getWindow());

        _renderer->present(_graphicsInterface->getWindow());
    }

    void SpriteEditor::handleEvents(const graphics::InputEvent& event) {
        switch (event.type) {
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
        case graphics::KEY_L:
            _isPlacingSprite = loadSpriteForPlacement("assets/water.dat");
            break;
        case graphics::KEY_K:
            saveSpriteToFile("sprite.dat");
            break;
        default:
            break;
        }
    }

    void SpriteEditor::imguiHandling() {
        _imguiInterface->spriteTopToolbar(_selectedTool, _brushSize, _isEraserActive);
        _imguiInterface->pixelSpriteHandler(_showDefaultPropertiesEditor, _newSpritePath);
        _imguiInterface->projectNavbar(_currentSpriteFilename);
        if (!_currentSpriteFilename.empty()) {
            _isPlacingSprite = loadSpriteForPlacement(_currentSpriteFilename);
            _currentSpriteFilename.clear();
        }
        if (!_newSpritePath.empty()) {
            saveSpriteToFile(_newSpritePath);
            _newSpritePath.clear();
        }        

        if (_showDefaultPropertiesEditor) {
            _imguiInterface->defaultPixelPropertiesEditor(_defaultPixelProperties, "Default Pixel Properties");
            _showPixelEditor = false;
        }

        if (_showPixelEditor) {
            // _showDefaultPropertiesEditor = false;
            // if (_currentPixel) {
                // _imguiInterface->pixelEditor(*_currentPixel, _chunkGrid, _pixelAttributes, "Pixel Editor");
            // }
        }
    }

    void SpriteEditor::mouseLeftClick() {
        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        if (_isPlacingSprite && _pendingSprite.valid) {
            placePendingSpriteAtWorld(worldPos);
            _isPlacingSprite = false;
            _pendingSprite = {};
            return;
        }

        const bool erase = (_selectedTool == 1) || _isEraserActive;
        applyBrushAt(worldPos, erase);
    }

    void SpriteEditor::mouseLeftDrag() {
        if (_isPlacingSprite) {
            return;
        }

        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);
        const bool erase = (_selectedTool == 1) || _isEraserActive;
        const bool drag = true;
        applyBrushAt(worldPos, erase, drag);
    }

    void SpriteEditor::applyBrushAt(glm::vec2 worldPos, bool erase, bool drag) {
        const int half = _brushSize / 2;
        const float centerX = std::round(worldPos.x / PIXEL_SIZE) * PIXEL_SIZE;
        const float centerY = std::round(worldPos.y / PIXEL_SIZE) * PIXEL_SIZE;

        for (int ox = -half; ox <= half; ++ox) {
            for (int oy = -half; oy <= half; ++oy) {
                glm::vec2 targetPos = {
                    centerX + (ox * PIXEL_SIZE),
                    centerY + (oy * PIXEL_SIZE)
                };

                graphics::Pixel* existing = getPixelAt(targetPos);
                if (erase) {
                    if (existing) {
                        removePixelAt(targetPos);
                    }
                    continue;
                }

                if (existing) {
                    if (drag) {
                        existing->color = _defaultPixelProperties.color;
                        _showPixelEditor = false;
                    } else {
                        _currentPixel = existing;
                        _showDefaultPropertiesEditor = false;
                        _showPixelEditor = true;
                    }
                    continue;
                }

                addPixel(targetPos, _defaultPixelProperties);
            }
        }
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
        for (auto& pixel : _renderPixels) {
            if (pixel.position == worldPos) {
                return &pixel;
            }
        }
        return nullptr;
    }

    bool SpriteEditor::removePixelAt(glm::vec2 worldPos) {
        // Find index of pixel to remove
        int removeIndex = -1;
        for (int i = 0; i < (int)_renderPixels.size(); ++i) {
            if (_renderPixels[i].position == worldPos) {
                removeIndex = i;
                break;
            }
        }
        if (removeIndex == -1) return false;

        // Get grid coords using same formula as getPixel
        int gridX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int gridY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        _chunkGrid.setPixel(gridX, gridY, Element::Pixel{Element::ElementType::EMPTY});

        // Remove from render list
        return true;
    }

    void SpriteEditor::addPixel(glm::vec2 worldPos, Pixel::DefaultPixelProperties defaultProperties) {
        _renderPixels.push_back({worldPos, defaultProperties.color});

        // Convert world position to grid coords first
        int gridX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int gridY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        // Then convert grid coords to chunk coords (same formula as getPixel/movePixel)
        int cx = (int)std::floor((float)gridX / CHUNK_SIZE);
        int cy = (int)std::floor((float)gridY / CHUNK_SIZE);

        // Local coords within chunk (0..31)
        int lx = ((gridX % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
        int ly = ((gridY % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;

        Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);
        chunk.set(lx, ly, Element::Pixel{Element::ElementType::EMPTY}); // TODO: use actual PixelEntityID and attributes


        _currentPixel = getPixelAt(worldPos);
    }


    bool SpriteEditor::saveSpriteToFile(const std::string& filename) {
        // TODO
        return true;
    }

    bool SpriteEditor::loadSpriteFromFile(const std::string& filename) {
        // TODO
        return true;
    }

    bool SpriteEditor::loadSpriteForPlacement(const std::string& filename) {
        // TODO
        return _pendingSprite.valid;
    }

    void SpriteEditor::placePendingSpriteAtWorld(glm::vec2 worldPos) {
        // TODO
    }

    std::vector<graphics::Pixel> SpriteEditor::buildRenderPixels()
    {
        std::vector<graphics::Pixel> result;
        result.reserve(10000); // avoid realloc (tune later)

        for (const auto& [key, chunk] : _chunkGrid.chunks)
        {
            int cx = key >> 32;
            int cy = key & 0xFFFFFFFF;

            for (int y = 0; y < CHUNK_SIZE; ++y)
            {
                for (int x = 0; x < CHUNK_SIZE; ++x)
                {
                    const Element::Pixel& simPixel = chunk.pixels[y * CHUNK_SIZE + x];

                    if (simPixel.type == Element::EMPTY)
                        continue;

                    const auto& def = g_elements[simPixel.type];

                    graphics::Pixel renderPixel;

                    // WORLD POSITION
                    renderPixel.position = glm::vec2(
                        cx * CHUNK_SIZE + x,
                        cy * CHUNK_SIZE + y
                    );

                    // COLOR (normalized 0–1)
                    renderPixel.color = glm::vec3(
                        def.color[0] / 255.0f,
                        def.color[1] / 255.0f,
                        def.color[2] / 255.0f
                    );

                    result.push_back(renderPixel);
                }
            }
        }

        return result;
    }

} // namespace editors