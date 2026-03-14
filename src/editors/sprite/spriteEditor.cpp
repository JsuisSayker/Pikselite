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

        std::vector<graphics::Pixel> framePixels = _renderPixels;

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
        _imguiInterface->pixelSpriteHandler(_showDefaultPropertiesEditor, _isEraserActive, _newSpritePath);
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
            _showDefaultPropertiesEditor = false;
            if (_currentPixel) {
                _imguiInterface->pixelEditor(*_currentPixel, _chunkGrid, _pixelAttributes, "Pixel Editor");
            }
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

        graphics::Pixel* pixel = getPixelAt(worldPos);
        if (pixel) {
            if (_isEraserActive) {
                removePixelAt(worldPos);
                return;
            }
            _currentPixel = pixel;
            _showDefaultPropertiesEditor = false;
            _showPixelEditor = true;
            return;
        }
        addPixel(worldPos, _defaultPixelProperties);
    }

    void SpriteEditor::mouseLeftDrag() {
        if (_isPlacingSprite) {
            return;
        }

        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        removePixelAt(worldPos);

        if (_isEraserActive) {
            return;
        }

        addPixel(worldPos, _defaultPixelProperties);
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

        Pixel::PixelEntityID removedId = _chunkGrid.getPixel(gridX, gridY);
        _chunkGrid.removePixel(gridX, gridY);
        removePixelAttributes(removedId);

        // Remove from render list
        _renderPixels.erase(_renderPixels.begin() + removeIndex);

        // Fix all renderIndex values that shifted due to erase
        for (auto& [id, index] : _pixelAttributes.renderIndex) {
            if (index > removeIndex) {
                index--;
            }
        }

        return true;
    }

    void SpriteEditor::addPixel(glm::vec2 worldPos, Pixel::DefaultPixelProperties defaultProperties) {
        _renderPixels.push_back({worldPos, defaultProperties.color});

        // Convert world position to grid coords first
        int gridX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int gridY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        // Then convert grid coords to chunk coords (same formula as getPixel/movePixel)
        int cx = (int)std::floor((float)gridX / Pixel::CHUNK_SIZE);
        int cy = (int)std::floor((float)gridY / Pixel::CHUNK_SIZE);

        // Local coords within chunk (0..31)
        int lx = ((gridX % Pixel::CHUNK_SIZE) + Pixel::CHUNK_SIZE) % Pixel::CHUNK_SIZE;
        int ly = ((gridY % Pixel::CHUNK_SIZE) + Pixel::CHUNK_SIZE) % Pixel::CHUNK_SIZE;

        Pixel::Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);
        chunk.set(lx, ly, pixelIdCounter);

        if (defaultProperties.isSolid)
            _pixelAttributes.solidAttributes[pixelIdCounter] = defaultProperties.solidAttributes;
        if (defaultProperties.isLiquid)
            _pixelAttributes.liquidAttributes[pixelIdCounter] = defaultProperties.liquidAttributes;
        if (defaultProperties.isGaseous)
            _pixelAttributes.gaseousAttributes[pixelIdCounter] = defaultProperties.gaseousAttributes;

        _pixelAttributes.renderIndex[pixelIdCounter] = (int)_renderPixels.size() - 1;
        pixelIdCounter++;

        _currentPixel = getPixelAt(worldPos);
    }


    bool SpriteEditor::saveSpriteToFile(const std::string& filename) {
        std::ofstream fout(filename, std::ios::binary);
        if (!fout) return false;

        uint32_t numChunks = static_cast<uint32_t>(_chunkGrid.getChunks().size());
        fout.write(reinterpret_cast<const char*>(&numChunks), sizeof(numChunks));

        for (const auto& [coord, chunk] : _chunkGrid.getChunks()) {
            int32_t cx = coord.first;
            int32_t cy = coord.second;

            fout.write(reinterpret_cast<const char*>(&cx), sizeof(cx));
            fout.write(reinterpret_cast<const char*>(&cy), sizeof(cy));

            for (int x = 0; x < Pixel::CHUNK_SIZE; ++x) {
                for (int y = 0; y < Pixel::CHUNK_SIZE; ++y) {
                    Pixel::PixelEntityID id = chunk.get(x, y);
                    fout.write(reinterpret_cast<const char*>(&id), sizeof(id));
                }
            }
        }

        uint32_t count = static_cast<uint32_t>(_pixelAttributes.renderIndex.size());
        fout.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& [id, index] : _pixelAttributes.renderIndex) {
            fout.write(reinterpret_cast<const char*>(&id), sizeof(id));
            fout.write(reinterpret_cast<const char*>(&index), sizeof(index));
        }

        count = static_cast<uint32_t>(_pixelAttributes.solidAttributes.size());
        fout.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& [id, solid] : _pixelAttributes.solidAttributes) {
            fout.write(reinterpret_cast<const char*>(&id), sizeof(id));
        }

        count = static_cast<uint32_t>(_pixelAttributes.liquidAttributes.size());
        fout.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& [id, liquid] : _pixelAttributes.liquidAttributes)
        {
            fout.write(reinterpret_cast<const char*>(&id), sizeof(id));
            fout.write(reinterpret_cast<const char*>(&liquid.viscosity), sizeof(liquid.viscosity));
        }

        count = static_cast<uint32_t>(_pixelAttributes.gaseousAttributes.size());
        fout.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& [id, gaseous] : _pixelAttributes.gaseousAttributes)
        {
            fout.write(reinterpret_cast<const char*>(&id), sizeof(id));
            fout.write(reinterpret_cast<const char*>(&gaseous.density), sizeof(gaseous.density));
        }

        uint32_t numPixels = static_cast<uint32_t>(_renderPixels.size());
        fout.write(reinterpret_cast<const char*>(&numPixels), sizeof(numPixels));
        for (const auto& pixel : _renderPixels) {
            float px = pixel.position.x;
            float py = pixel.position.y;
            float r = pixel.color.r;
            float g = pixel.color.g;
            float b = pixel.color.b;

            fout.write(reinterpret_cast<const char*>(&px), sizeof(px));
            fout.write(reinterpret_cast<const char*>(&py), sizeof(py));
            fout.write(reinterpret_cast<const char*>(&r), sizeof(r));
            fout.write(reinterpret_cast<const char*>(&g), sizeof(g));
            fout.write(reinterpret_cast<const char*>(&b), sizeof(b));
        }

        fout.close();
        return true;
    }

    bool SpriteEditor::loadSpriteFromFile(const std::string& filename) {
        std::ifstream fin(filename, std::ios::binary);
        if (!fin) return false;
        _chunkGrid = Pixel::ChunkGrid();
        _renderPixels.clear();

        uint32_t numChunks = 0;
        fin.read(reinterpret_cast<char*>(&numChunks), sizeof(numChunks));
        for (uint32_t i = 0; i < numChunks; ++i)
        {
            int32_t cx = 0;
            int32_t cy = 0;
            fin.read(reinterpret_cast<char*>(&cx), sizeof(cx));
            fin.read(reinterpret_cast<char*>(&cy), sizeof(cy));

            Pixel::Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);
            for (int x = 0; x < Pixel::CHUNK_SIZE; ++x) {
                for (int y = 0; y < Pixel::CHUNK_SIZE; ++y) {
                    Pixel::PixelEntityID id = Pixel::EMPTY;
                    fin.read(reinterpret_cast<char*>(&id), sizeof(id));
                    chunk.set(x, y, id);
                }
            }
        }

        uint32_t count = 0;
        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            int index = 0;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            fin.read(reinterpret_cast<char*>(&index), sizeof(index));
            _pixelAttributes.renderIndex[id] = index;
        }

        count = 0;
        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            _pixelAttributes.solidAttributes[id] = Pixel::Solid();
        }

        count = 0;
        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            float viscosity = 0.0f;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            fin.read(reinterpret_cast<char*>(&viscosity), sizeof(viscosity));
            _pixelAttributes.liquidAttributes[id] = Pixel::Liquid{viscosity};
        }

        count = 0;
        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            float density = 0.0f;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            fin.read(reinterpret_cast<char*>(&density), sizeof(density));
            _pixelAttributes.gaseousAttributes[id] = Pixel::Gaseous{density};
        }

        uint32_t numPixels = 0;
        fin.read(reinterpret_cast<char*>(&numPixels), sizeof(numPixels));
        for (uint32_t i = 0; i < numPixels; ++i) {
            float px = 0.0f;
            float py = 0.0f;
            float r = 0.0f;
            float g = 0.0f;
            float b = 0.0f;

            fin.read(reinterpret_cast<char*>(&px), sizeof(px));
            fin.read(reinterpret_cast<char*>(&py), sizeof(py));
            fin.read(reinterpret_cast<char*>(&r), sizeof(r));
            fin.read(reinterpret_cast<char*>(&g), sizeof(g));
            fin.read(reinterpret_cast<char*>(&b), sizeof(b));

            _renderPixels.push_back({{px, py}, {r, g, b}});
        }

        fin.close();
        pixelIdCounter = _renderPixels.size() + 1;
        return true;
    }

    void SpriteEditor::removePixelAttributes(Pixel::PixelEntityID id) {
        _pixelAttributes.renderIndex.erase(id);
        _pixelAttributes.solidAttributes.erase(id);
        _pixelAttributes.liquidAttributes.erase(id);
        _pixelAttributes.gaseousAttributes.erase(id);
    }

    bool SpriteEditor::loadSpriteForPlacement(const std::string& filename) {
        std::ifstream fin(filename, std::ios::binary);
        if (!fin) return false;

        PendingSprite pending{};
        std::cout << "Loading sprite for placement: " << filename << std::endl;

        uint32_t numChunks = 0;
        fin.read(reinterpret_cast<char*>(&numChunks), sizeof(numChunks));

        int minGX = std::numeric_limits<int>::max();
        int minGY = std::numeric_limits<int>::max();

        for (uint32_t i = 0; i < numChunks; ++i) {
            int32_t cx = 0;
            int32_t cy = 0;
            fin.read(reinterpret_cast<char*>(&cx), sizeof(cx));
            fin.read(reinterpret_cast<char*>(&cy), sizeof(cy));

            for (int x = 0; x < Pixel::CHUNK_SIZE; ++x) {
                for (int y = 0; y < Pixel::CHUNK_SIZE; ++y) {
                    Pixel::PixelEntityID id = Pixel::EMPTY;
                    fin.read(reinterpret_cast<char*>(&id), sizeof(id));
                    if (id == Pixel::EMPTY) continue;

                    const int gx = cx * Pixel::CHUNK_SIZE + x;
                    const int gy = cy * Pixel::CHUNK_SIZE + y;

                    minGX = std::min(minGX, gx);
                    minGY = std::min(minGY, gy);

                    pending.cells.push_back({gx, gy, id});
                }
            }
        }

        uint32_t count = 0;

        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            int index = 0;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            fin.read(reinterpret_cast<char*>(&index), sizeof(index));
            pending.oldRenderIndex[id] = index;
        }

        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            pending.solids[id] = Pixel::Solid{};
        }

        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            float viscosity = 0.0f;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            fin.read(reinterpret_cast<char*>(&viscosity), sizeof(viscosity));
            pending.liquids[id] = Pixel::Liquid{viscosity};
        }

        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            float density = 0.0f;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            fin.read(reinterpret_cast<char*>(&density), sizeof(density));
            pending.gases[id] = Pixel::Gaseous{density};
        }

        uint32_t numPixels = 0;
        fin.read(reinterpret_cast<char*>(&numPixels), sizeof(numPixels));
        pending.loadedRenderPixels.reserve(numPixels);

        for (uint32_t i = 0; i < numPixels; ++i) {
            float px = 0.0f;
            float py = 0.0f;
            float r = 0.0f;
            float g = 0.0f;
            float b = 0.0f;

            fin.read(reinterpret_cast<char*>(&px), sizeof(px));
            fin.read(reinterpret_cast<char*>(&py), sizeof(py));
            fin.read(reinterpret_cast<char*>(&r), sizeof(r));
            fin.read(reinterpret_cast<char*>(&g), sizeof(g));
            fin.read(reinterpret_cast<char*>(&b), sizeof(b));

            pending.loadedRenderPixels.push_back({{px, py}, {r, g, b}});
        }

        for (auto& c : pending.cells) {
            c.localGX -= minGX;
            c.localGY -= minGY;

            glm::vec3 color{1.0f, 1.0f, 1.0f};
            auto it = pending.oldRenderIndex.find(c.oldId);
            if (it != pending.oldRenderIndex.end()) {
                const int index = it->second;
                if (index >= 0 && index < static_cast<int>(pending.loadedRenderPixels.size())) {
                    color = pending.loadedRenderPixels[index].color;
                }
            }

            pending.previewLocalPixels.push_back({
                {c.localGX * PIXEL_SIZE, c.localGY * PIXEL_SIZE},
                color
            });
        }

        pending.valid = !pending.cells.empty();
        _pendingSprite = std::move(pending);
        return _pendingSprite.valid;
    }

    void SpriteEditor::placePendingSpriteAtWorld(glm::vec2 worldPos) {
        if (!_pendingSprite.valid) return;

        auto toChunk = [](int g) -> int {
            return (g >= 0) ? (g / Pixel::CHUNK_SIZE) : ((g - Pixel::CHUNK_SIZE + 1) / Pixel::CHUNK_SIZE);
        };

        auto toLocal = [](int g) -> int {
            return ((g % Pixel::CHUNK_SIZE) + Pixel::CHUNK_SIZE) % Pixel::CHUNK_SIZE;
        };

        const int anchorGX = static_cast<int>(std::floor(worldPos.x / PIXEL_SIZE));
        const int anchorGY = static_cast<int>(std::floor(worldPos.y / PIXEL_SIZE));

        for (const auto& c : _pendingSprite.cells) {
            const int gx = anchorGX + c.localGX;
            const int gy = anchorGY + c.localGY;

            const int cx = toChunk(gx);
            const int cy = toChunk(gy);
            const int lx = toLocal(gx);
            const int ly = toLocal(gy);

            const Pixel::PixelEntityID newId = pixelIdCounter++;

            Pixel::Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);
            chunk.set(lx, ly, newId);

            if (_pendingSprite.solids.count(c.oldId))
                _pixelAttributes.solidAttributes[newId] = _pendingSprite.solids[c.oldId];
            if (_pendingSprite.liquids.count(c.oldId))
                _pixelAttributes.liquidAttributes[newId] = _pendingSprite.liquids[c.oldId];
            if (_pendingSprite.gases.count(c.oldId))
                _pixelAttributes.gaseousAttributes[newId] = _pendingSprite.gases[c.oldId];

            glm::vec3 color{1.0f, 1.0f, 1.0f};
            auto it = _pendingSprite.oldRenderIndex.find(c.oldId);
            if (it != _pendingSprite.oldRenderIndex.end()) {
                const int index = it->second;
                if (index >= 0 && index < static_cast<int>(_pendingSprite.loadedRenderPixels.size())) {
                    color = _pendingSprite.loadedRenderPixels[index].color;
                }
            }

            _renderPixels.push_back({{gx * PIXEL_SIZE, gy * PIXEL_SIZE}, color});
            _pixelAttributes.renderIndex[newId] = static_cast<int>(_renderPixels.size()) - 1;
        }
    }

} // namespace editors