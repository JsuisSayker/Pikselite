#include <editors/sprite/spriteEditor.hpp>
#include <algorithm>

namespace editors {

    SpriteEditor::SpriteEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface)
        : _graphicsInterface(graphicsInterface), _renderer(renderer), _imguiInterface(imguiInterface) {}

    SpriteEditor::~SpriteEditor() {}

    void SpriteEditor::run(const graphics::InputEvent& event) {
        handleEvents(event);

        _renderer->clear();
        _imguiInterface->startFrame();

        _renderer->drawPixelsWCamera(_renderPixels, _camera, PIXEL_SIZE);
        _renderer->drawGrid(_camera, PIXEL_SIZE, {0.7f, 0.7f, 0.7f}); // Draw grid with cell size 16

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
            loadSpriteFromFile("sprite.dat");
            break;
        case graphics::KEY_K:
            saveSpriteToFile("sprite.dat");
            break;
        default:
            break;
        }
    }

    void SpriteEditor::imguiHandling() {
        _imguiInterface->showImGuiDemo();
        if (_showPixelEditor) {
            if (_currentPixel) {
                _imguiInterface->pixelEditor(*_currentPixel, _chunkGrid, _pixelAttributes, "Pixel Editor");
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
        for (auto& pixel : _renderPixels) {
            if (pixel.position == worldPos) {
                return &pixel;
            }
        }
        return nullptr;
    }

    bool SpriteEditor::removePixelAt(glm::vec2 worldPos) {
        auto it = std::remove_if(_renderPixels.begin(), _renderPixels.end(),
                                 [&worldPos](const graphics::Pixel& pixel) {
                                     return pixel.position == worldPos;
                                 });
        if (it != _renderPixels.end()) {
            _renderPixels.erase(it, _renderPixels.end());
            _chunkGrid.removePixel((int)worldPos.x, (int)worldPos.y);
            return true;
        }
        return false;
    }

    void SpriteEditor::addPixel(glm::vec2 worldPos, float r, float g, float b) {
        _renderPixels.push_back({worldPos, {r, g, b}});
        int cx = (int)std::floor(worldPos.x / Pixel::CHUNK_SIZE);
        int cy = (int)std::floor(worldPos.y / Pixel::CHUNK_SIZE);
        Pixel::Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);
        
        // Safe modulo for negative coordinates
        int lx = ((int)worldPos.x % Pixel::CHUNK_SIZE + Pixel::CHUNK_SIZE) % Pixel::CHUNK_SIZE;
        int ly = ((int)worldPos.y % Pixel::CHUNK_SIZE + Pixel::CHUNK_SIZE) % Pixel::CHUNK_SIZE;
        
        chunk.set(lx, ly, pixelIdCounter);
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

} // namespace editors