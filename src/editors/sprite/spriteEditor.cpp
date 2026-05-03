#include <algorithm>
#include <editors/sprite/spriteEditor.hpp>

namespace editors
{

    SpriteEditor::SpriteEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer,
                               graphics::ImguiInterface* imguiInterface)
        : AbstractEditor(graphicsInterface, renderer, imguiInterface)
    {
    }

    SpriteEditor::~SpriteEditor() {}

    void SpriteEditor::run(const graphics::InputEvent& event)
    {
        handleEvents(event);

        _renderer->clear();
        _imguiInterface->startFrame();

        std::vector<graphics::Pixel> framePixels         = buildRenderPixels();
        std::vector<graphics::Pixel> pendingSpritePixels = addPendingSpriteToRenderPixels();
        framePixels.insert(framePixels.end(), pendingSpritePixels.begin(),
                           pendingSpritePixels.end());
        _renderPixels = framePixels; // cache for editing

        _renderer->drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);
        _renderer->drawGrid(_camera, PIXEL_SIZE, {0.7f, 0.7f, 0.7f});

        imguiHandling();
        _imguiInterface->endFrame(_graphicsInterface->getWindow());

        _renderer->present(_graphicsInterface->getWindow());
    }

    void SpriteEditor::handleEvents(const graphics::InputEvent& event)
    {
        switch (event.type)
        {
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

    void SpriteEditor::imguiHandling()
    {
        _imguiInterface->spriteTopToolbar(_selectedTool, _brushSize, _isEraserActive);

        // Default properties panel removed
        bool unusedDefaultPropertiesEditor = false;
        _imguiInterface->pixelSpriteHandler(unusedDefaultPropertiesEditor, _newSpritePath,
                                            _currentElementType);

        _imguiInterface->projectNavbar(_currentSpriteFilename);

        if (!_currentSpriteFilename.empty())
        {
            _isPlacingSprite = loadSpriteForPlacement(_currentSpriteFilename);
            _currentSpriteFilename.clear();
        }

        if (!_newSpritePath.empty())
        {
            saveSpriteToFile(_newSpritePath);
            _newSpritePath.clear();
        }

        // Removed _showDefaultPropertiesEditor / _showPixelEditor blocks
    }

    Element::ElementType SpriteEditor::getElementTypeAt(glm::vec2 worldPos)
    {
        const int gridX = static_cast<int>(std::floor(worldPos.x / PIXEL_SIZE));
        const int gridY = static_cast<int>(std::floor(worldPos.y / PIXEL_SIZE));
        return _chunkGrid.getPixel(gridX, gridY).type;
    }

    void SpriteEditor::mouseLeftClick()
    {
        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        if (_isPlacingSprite && _pendingSprite.valid)
        {
            placePendingSpriteAtWorld(worldPos);
            _isPlacingSprite = false;
            _pendingSprite   = {};
            return;
        }

        const bool erase = (_selectedTool == 1) || _isEraserActive;

        applyBrushAt(worldPos, erase);
    }

    void SpriteEditor::mouseLeftDrag()
    {
        if (_isPlacingSprite)
        {
            return;
        }

        glm::vec2  mousePos = _graphicsInterface->getMousePosition();
        glm::vec2  worldPos = screenToWorld(mousePos);
        const bool erase    = (_selectedTool == 1) || _isEraserActive;
        const bool drag     = true;
        applyBrushAt(worldPos, erase, drag);
    }

    void SpriteEditor::applyBrushAt(glm::vec2 worldPos, bool erase, bool drag)
    {
        const int   half    = _brushSize / 2;
        const float centerX = std::round(worldPos.x / PIXEL_SIZE) * PIXEL_SIZE;
        const float centerY = std::round(worldPos.y / PIXEL_SIZE) * PIXEL_SIZE;

        for (int ox = -half; ox <= half; ++ox)
        {
            for (int oy = -half; oy <= half; ++oy)
            {
                glm::vec2 targetPos = {centerX + (ox * PIXEL_SIZE), centerY + (oy * PIXEL_SIZE)};

                graphics::Pixel* existing = getPixelAt(targetPos);

                if (erase)
                {
                    if (existing)
                        removePixelAt(targetPos);
                    continue;
                }

                addPixel(targetPos);

                // Removed editor panel toggles
                (void)drag;
            }
        }
    }

    graphics::Pixel* SpriteEditor::getPixelAt(glm::vec2 worldPos)
    {
        for (auto& pixel : _renderPixels)
        {
            if (pixel.position == worldPos)
            {
                return &pixel;
            }
        }
        return nullptr;
    }

    bool SpriteEditor::removePixelAt(glm::vec2 worldPos)
    {
        // Get grid coords using same formula as getPixel
        int gridX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int gridY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        _chunkGrid.setPixel(gridX, gridY, Element::Pixel{Element::ElementType::EMPTY});

        // Remove from render list
        return true;
    }

    void SpriteEditor::addPixel(glm::vec2 worldPos)
    {

        // Convert world position to grid coords first
        int gridX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int gridY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        // Then convert grid coords to chunk coords (same formula as getPixel/movePixel)
        int cx = (int)std::floor((float)gridX / CHUNK_SIZE);
        int cy = (int)std::floor((float)gridY / CHUNK_SIZE);

        // Local coords within chunk (0..31)
        int lx = ((gridX % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
        int ly = ((gridY % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;

        // colorIndex can be used for simple variation based on position, or could be extended to
        // use a more complex palette system in the future
        uint8_t colorIndex = _renderer->generatePixelColorIndex(gridX, gridY);

        Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);
        chunk.set(lx, ly,
                  Element::Pixel{_currentElementType, false, colorIndex}); // use selected element

        _currentPixel = getPixelAt(worldPos);
    }

    bool SpriteEditor::saveSpriteToFile(const std::string& filename)
    {
        std::ofstream fout(filename, std::ios::binary);
        if (!fout)
        {
            std::cerr << "Failed to open file for saving: " << filename << std::endl;
            return false;
        }

        uint32_t nbChunks = _chunkGrid.chunks.size();
        fout.write(reinterpret_cast<const char*>(&nbChunks), sizeof(nbChunks));

        for (const auto& [key, chunk] : _chunkGrid.chunks)
        {
            // Decode chunk coords from key
            const int32_t cx = static_cast<int32_t>(key >> 32);
            const int32_t cy = static_cast<int32_t>(key & 0xFFFFFFFF);

            // Save coords
            fout.write(reinterpret_cast<const char*>(&cx), sizeof(cx));
            fout.write(reinterpret_cast<const char*>(&cy), sizeof(cy));

            // Save only the pixels, NOT updatedThisFrame
            for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i)
            {
                fout.write(reinterpret_cast<const char*>(&chunk.pixels[i].type),
                           sizeof(Element::ElementType));
            }
        }

        return true;
    }

    void SpriteEditor::placePendingSpriteAtWorld(glm::vec2 worldPos)
    {
        if (!_pendingSprite.valid)
            return;

        // Convert world position to grid coords
        int anchorGX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int anchorGY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        // Place each cell relative to anchor
        for (const auto& cell : _pendingSprite.cells)
        {
            int targetGX = anchorGX + cell.localGX;
            int targetGY = anchorGY + cell.localGY;

            int cx = toChunk(targetGX);
            int cy = toChunk(targetGY);
            int lx = toLocal(targetGX);
            int ly = toLocal(targetGY);

            Chunk&  chunk      = _chunkGrid.getOrCreateChunk(cx, cy);
            uint8_t colorIndex = _renderer->generatePixelColorIndex(targetGX, targetGY);
            chunk.set(lx, ly, Element::Pixel{cell.type, false, colorIndex});
        }
    }
} // namespace editors