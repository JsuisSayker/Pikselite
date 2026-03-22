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
        std::vector<graphics::Pixel> pendingSpritePixels = addPendingSpriteToRenderPixels();
        framePixels.insert(framePixels.end(), pendingSpritePixels.begin(), pendingSpritePixels.end());
        _renderPixels = framePixels; // cache for editing

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
        // Get grid coords using same formula as getPixel
        int gridX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int gridY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        _chunkGrid.setPixel(gridX, gridY, Element::Pixel{Element::ElementType::EMPTY});

        // Remove from render list
        return true;
    }

    void SpriteEditor::addPixel(glm::vec2 worldPos, Pixel::DefaultPixelProperties defaultProperties) {
        
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
        chunk.set(lx, ly, Element::Pixel{Element::ElementType::SAND}); // TODO: use actual PixelEntityID and attributes


        _currentPixel = getPixelAt(worldPos);
    }


    bool SpriteEditor::saveSpriteToFile(const std::string& filename) {
        std::ofstream fout(filename, std::ios::binary);
        if (!fout) {
            std::cerr << "Failed to open file for saving: " << filename << std::endl;
            return false;
        }

        uint32_t nbChunks = _chunkGrid.chunks.size();
        fout.write(reinterpret_cast<const char*>(&nbChunks), sizeof(nbChunks));

        for (const auto& [key, chunk] : _chunkGrid.chunks) {
            // Decode chunk coords from key
            const int32_t cx = static_cast<int32_t>(key >> 32);
            const int32_t cy = static_cast<int32_t>(key & 0xFFFFFFFF);

            // Save coords
            fout.write(reinterpret_cast<const char*>(&cx), sizeof(cx));
            fout.write(reinterpret_cast<const char*>(&cy), sizeof(cy));

            // Save only the pixels, NOT updatedThisFrame
            for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
                fout.write(reinterpret_cast<const char*>(&chunk.pixels[i].type), sizeof(Element::ElementType));
            }
        }

        std::cout << "Saved sprite to: " << filename << std::endl;
        return true;
    }

    bool SpriteEditor::loadSpriteFromFile(const std::string& filename) {
        std::ifstream fin(filename, std::ios::binary);
        if (!fin) {
            std::cerr << "Failed to open file for loading: " << filename << std::endl;
            return false;
        }

        _chunkGrid.chunks.clear();

        uint32_t nbChunks = 0;
        fin.read(reinterpret_cast<char*>(&nbChunks), sizeof(nbChunks));

        for (uint32_t i = 0; i < nbChunks; ++i) {
            int32_t cx = 0, cy = 0;
            fin.read(reinterpret_cast<char*>(&cx), sizeof(cx));
            fin.read(reinterpret_cast<char*>(&cy), sizeof(cy));

            Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);

            for (int j = 0; j < CHUNK_SIZE * CHUNK_SIZE; ++j) {
                Element::ElementType type = Element::EMPTY;
                fin.read(reinterpret_cast<char*>(&type), sizeof(type));
                chunk.pixels[j].type = type;
                chunk.pixels[j].updatedThisFrame = false;
            }
        }

        std::cout << "Loaded sprite from: " << filename << std::endl;
        return true;
    }

    bool SpriteEditor::loadSpriteForPlacement(const std::string& filename) {
        _pendingSprite = {};
        _isPlacingSprite = false;
        
        std::ifstream fin(filename, std::ios::binary);
        if (!fin) {
            std::cerr << "Failed to open file for loading: " << filename << std::endl;
            return false;
        }

        PendingSprite pending{};
        int minGX = std::numeric_limits<int>::max();
        int minGY = std::numeric_limits<int>::max();
        std::vector<std::pair<std::pair<int32_t, int32_t>, std::vector<std::pair<int, Element::ElementType>>>> allChunks;

        uint32_t nbChunks = 0;
        fin.read(reinterpret_cast<char*>(&nbChunks), sizeof(nbChunks));

        // First pass: load and find min coords
        for (uint32_t i = 0; i < nbChunks; ++i) {
            int32_t cx = 0, cy = 0;
            fin.read(reinterpret_cast<char*>(&cx), sizeof(cx));
            fin.read(reinterpret_cast<char*>(&cy), sizeof(cy));

            std::vector<std::pair<int, Element::ElementType>> pixelsInChunk;

            for (int j = 0; j < CHUNK_SIZE * CHUNK_SIZE; ++j) {
                Element::ElementType type = Element::EMPTY;
                fin.read(reinterpret_cast<char*>(&type), sizeof(type));
                
                if (type != Element::ElementType::EMPTY) {
                    int lx = j % CHUNK_SIZE;
                    int ly = j / CHUNK_SIZE;
                    int gx = cx * CHUNK_SIZE + lx;
                    int gy = cy * CHUNK_SIZE + ly;

                    minGX = std::min(minGX, gx);
                    minGY = std::min(minGY, gy);

                    pixelsInChunk.push_back({j, type});
                }
            }

            allChunks.push_back({{cx, cy}, pixelsInChunk});
        }

        if (allChunks.empty()) {
            std::cerr << "No pixels in sprite file: " << filename << std::endl;
            return false;
        }

        // Second pass: normalize to local coords
        for (const auto& [coords, pixelsInChunk] : allChunks) {
            int32_t cx = coords.first;
            int32_t cy = coords.second;

            for (const auto& [j, type] : pixelsInChunk) {
                int lx = j % CHUNK_SIZE;
                int ly = j / CHUNK_SIZE;
                int gx = cx * CHUNK_SIZE + lx;
                int gy = cy * CHUNK_SIZE + ly;

                // Normalize to local coords relative to min
                int localGX = gx - minGX;
                int localGY = gy - minGY;

                pending.cells.push_back({localGX, localGY, type});
            }
        }

        pending.valid = !pending.cells.empty();
        _pendingSprite = std::move(pending);

        std::cout << "Loaded sprite for placement: " << filename << " (" << _pendingSprite.cells.size() << " pixels)" << std::endl;
        return _pendingSprite.valid;
    }

    void SpriteEditor::placePendingSpriteAtWorld(glm::vec2 worldPos) {
        if (!_pendingSprite.valid) return;

        // Convert world position to grid coords
        int anchorGX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int anchorGY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        auto toChunk = [](int g) -> int {
            return (g >= 0) ? (g / CHUNK_SIZE) : ((g - CHUNK_SIZE + 1) / CHUNK_SIZE);
        };

        auto toLocal = [](int g) -> int {
            return ((g % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
        };

        // Place each cell relative to anchor
        for (const auto& cell : _pendingSprite.cells) {
            int targetGX = anchorGX + cell.localGX;
            int targetGY = anchorGY + cell.localGY;

            int cx = toChunk(targetGX);
            int cy = toChunk(targetGY);
            int lx = toLocal(targetGX);
            int ly = toLocal(targetGY);

            Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);
            chunk.set(lx, ly, Element::Pixel{cell.type});
        }

        std::cout << "Placed sprite at grid: (" << anchorGX << ", " << anchorGY << ")" << std::endl;
    }

    std::vector<graphics::Pixel> SpriteEditor::addPendingSpriteToRenderPixels() {
        std::vector<graphics::Pixel> pendingSpritePixels;

        if (!_pendingSprite.valid) return pendingSpritePixels;

        // Get current mouse position in world coords
        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        // Convert to grid coords
        int anchorGX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int anchorGY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        for (const auto& cell : _pendingSprite.cells) {
            // Apply anchor offset to each cell
            int targetGX = anchorGX + cell.localGX;
            int targetGY = anchorGY + cell.localGY;

            float worldX = targetGX * PIXEL_SIZE;
            float worldY = targetGY * PIXEL_SIZE;

            graphics::Pixel renderPixel;
            renderPixel.position = glm::vec2(worldX, worldY);

            const auto& def = g_elements[cell.type];
            renderPixel.color = glm::vec3(
                def.color[0] / 255.0f,
                def.color[1] / 255.0f,
                def.color[2] / 255.0f
            );
            renderPixel.color *= 0.65f; // Ghost effect

            pendingSpritePixels.push_back(renderPixel);
        }
        return pendingSpritePixels;
    }

    std::vector<graphics::Pixel> SpriteEditor::buildRenderPixels()
    {
        std::vector<graphics::Pixel> result;
        result.reserve(10000);

        for (const auto& [key, chunk] : _chunkGrid.chunks)
        {
            // Correct signed decode from packed int64 key
            const int cx = static_cast<int32_t>(key >> 32);
            const int cy = static_cast<int32_t>(key & 0xFFFFFFFF);

            for (int y = 0; y < CHUNK_SIZE; ++y)
            {
                for (int x = 0; x < CHUNK_SIZE; ++x)
                {
                    const Element::Pixel& simPixel = chunk.pixels[y * CHUNK_SIZE + x];
                    if (simPixel.type == Element::EMPTY) continue;

                    const auto& def = g_elements[simPixel.type];

                    graphics::Pixel renderPixel;

                    // grid -> world (apply chunk offset + pixel size)
                    const float gx = static_cast<float>(cx * CHUNK_SIZE + x);
                    const float gy = static_cast<float>(cy * CHUNK_SIZE + y);

                    renderPixel.position = glm::vec2(
                        gx * PIXEL_SIZE,
                        gy * PIXEL_SIZE
                    );

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