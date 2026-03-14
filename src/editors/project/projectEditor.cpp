#include <editors/project/projectEditor.hpp>
#include <SDL2/SDL.h>

namespace editors {

    ProjectEditor::ProjectEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface)
        : _graphicsInterface(graphicsInterface), _renderer(renderer), _imguiInterface(imguiInterface) {}

    ProjectEditor::~ProjectEditor() {}

    void ProjectEditor::run(const graphics::InputEvent& event) {
        handleEvents(event);
        updatePlacementMode();

        _renderer->clear();

        _imguiInterface->startFrame();

        std::vector<graphics::Pixel> framePixels = _renderPixels;

        // ghost preview while placing
        if (_isPlacingSprite && _pendingSprite.valid) {
            glm::vec2 mouse = _graphicsInterface->getMousePosition();
            glm::vec2 world = screenToWorld(mouse);

            int anchorGX = static_cast<int>(std::floor(world.x / PIXEL_SIZE));
            int anchorGY = static_cast<int>(std::floor(world.y / PIXEL_SIZE));

            for (const auto& p : _pendingSprite.previewLocalPixels) {
                graphics::Pixel ghost = p;
                ghost.position.x = anchorGX * PIXEL_SIZE + p.position.x;
                ghost.position.y = anchorGY * PIXEL_SIZE + p.position.y;
                ghost.color *= 0.65f; // visual ghost
                framePixels.push_back(ghost);
            }
        }

        _renderer->drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);
        _renderer->drawGrid(_camera, PIXEL_SIZE, {0.7f, 0.7f, 0.7f});
        imguiHandling();
        _imguiInterface->endFrame(_graphicsInterface->getWindow());
        _renderer->present(_graphicsInterface->getWindow());
    }

    void ProjectEditor::handleEvents(const graphics::InputEvent& event) {
        switch (event.type) {
        case graphics::KEY_W:
            _camera.move(glm::vec2(0.0f, -10.0f));
            break;
        case graphics::KEY_S:
            _camera.move(glm::vec2(0.0f, 10.0f));
            break;
        case graphics::KEY_A:
            _camera.move(glm::vec2(10.0f, 0.0f));
            break;
        case graphics::KEY_D:
            _camera.move(glm::vec2(-10.0f, 0.0f));
            break;
        case graphics::KEY_I:
            _camera.zoomIn(1.1f);
            break;
        case graphics::KEY_O:
            _camera.zoomOut(1.1f);
            break;
        case graphics::KEY_L:
            // Start placement mode instead of direct load
            loadSpriteForPlacement("assets/water.dat");
            _isPlacingSprite = _pendingSprite.valid;
            break;
        default:
            break;
        }
    }

    glm::vec2 ProjectEditor::screenToWorld(const glm::vec2& screenPos) const {
        // Adjust this call if your Camera2D API name differs
        int w = 0, h = 0;
        SDL_GetWindowSize(_graphicsInterface->getWindow(), &w, &h);
        return _camera.screenToWorld(screenPos, w, h);
    }

    void ProjectEditor::updatePlacementMode() {
        if (!_isPlacingSprite || !_pendingSprite.valid) return;

        const bool leftDown = (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
        if (leftDown && !_leftMouseDownLastFrame) {
            glm::vec2 mouse = _graphicsInterface->getMousePosition();
            glm::vec2 world = screenToWorld(mouse);

            int anchorGX = static_cast<int>(std::floor(world.x / PIXEL_SIZE));
            int anchorGY = static_cast<int>(std::floor(world.y / PIXEL_SIZE));

            placePendingSpriteAtGrid(anchorGX, anchorGY);

            _isPlacingSprite = false;
            _pendingSprite = {};
        }
        _leftMouseDownLastFrame = leftDown;
    }

    bool ProjectEditor::loadSpriteForPlacement(const std::string& filename) {
        std::ifstream fin(filename, std::ios::binary);
        if (!fin) return false;

        PendingSprite pending{};

        uint32_t numChunks = 0;
        fin.read(reinterpret_cast<char*>(&numChunks), sizeof(numChunks));

        int minGX = std::numeric_limits<int>::max();
        int minGY = std::numeric_limits<int>::max();

        for (uint32_t i = 0; i < numChunks; ++i) {
            int32_t cx = 0, cy = 0;
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

                    pending.cells.push_back({gx, gy, id}); // temp global, normalized later
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
            float px = 0, py = 0, r = 0, g = 0, b = 0;
            fin.read(reinterpret_cast<char*>(&px), sizeof(px));
            fin.read(reinterpret_cast<char*>(&py), sizeof(py));
            fin.read(reinterpret_cast<char*>(&r), sizeof(r));
            fin.read(reinterpret_cast<char*>(&g), sizeof(g));
            fin.read(reinterpret_cast<char*>(&b), sizeof(b));
            pending.loadedRenderPixels.push_back({{px, py}, {r, g, b}});
        }

        // normalize cells + build local preview pixels
        for (auto& c : pending.cells) {
            c.localGX -= minGX;
            c.localGY -= minGY;

            glm::vec3 color{1.0f, 1.0f, 1.0f};
            auto itIdx = pending.oldRenderIndex.find(c.oldId);
            if (itIdx != pending.oldRenderIndex.end() &&
                itIdx->second >= 0 &&
                itIdx->second < static_cast<int>(pending.loadedRenderPixels.size())) {
                color = pending.loadedRenderPixels[itIdx->second].color;
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

    void ProjectEditor::placePendingSpriteAtGrid(int anchorGX, int anchorGY) {
        if (!_pendingSprite.valid) return;

        Pixel::GameObject obj;
        obj.id = gameObjectCounter++;
        obj.name = "GameObject"; ////////////////////////////////// placeholder name

        auto toChunk = [](int g) -> int {
            return (g >= 0) ? (g / Pixel::CHUNK_SIZE) : ((g - Pixel::CHUNK_SIZE + 1) / Pixel::CHUNK_SIZE);
        };
        auto toLocal = [](int g) -> int {
            return ((g % Pixel::CHUNK_SIZE) + Pixel::CHUNK_SIZE) % Pixel::CHUNK_SIZE;
        };

        for (const auto& c : _pendingSprite.cells) {
            const int gx = anchorGX + c.localGX;
            const int gy = anchorGY + c.localGY;

            const int cx = toChunk(gx);
            const int cy = toChunk(gy);
            const int lx = toLocal(gx);
            const int ly = toLocal(gy);

            const Pixel::PixelEntityID newId = pixelIdCounter++;
            _chunkGrid.getOrCreateChunk(cx, cy).set(lx, ly, newId);
            obj.pixelEntities.push_back(newId);

            // copy material attrs from old id -> new id
            if (_pendingSprite.solids.count(c.oldId))  _pixelAttributes.solidAttributes[newId] = _pendingSprite.solids[c.oldId];
            if (_pendingSprite.liquids.count(c.oldId)) _pixelAttributes.liquidAttributes[newId] = _pendingSprite.liquids[c.oldId];
            if (_pendingSprite.gases.count(c.oldId))   _pixelAttributes.gaseousAttributes[newId] = _pendingSprite.gases[c.oldId];

            glm::vec3 color{1.0f, 1.0f, 1.0f};
            auto itIdx = _pendingSprite.oldRenderIndex.find(c.oldId);
            if (itIdx != _pendingSprite.oldRenderIndex.end() &&
                itIdx->second >= 0 &&
                itIdx->second < static_cast<int>(_pendingSprite.loadedRenderPixels.size())) {
                color = _pendingSprite.loadedRenderPixels[itIdx->second].color;
            }

            _renderPixels.push_back({{gx * PIXEL_SIZE, gy * PIXEL_SIZE}, color});
            _pixelAttributes.renderIndex[newId] = static_cast<int>(_renderPixels.size()) - 1;
        }

        _gameObjects.push_back(std::move(obj));
    }

    void ProjectEditor::imguiHandling()
    {
        _imguiInterface->gameObjectsBar(_gameObjects);

        _imguiInterface->projectNavbar(_currentSpriteFilename);
        if (!_currentSpriteFilename.empty()) {
            _isPlacingSprite = loadSpriteForPlacement(_currentSpriteFilename);
            _currentSpriteFilename.clear();
        }
    }

} // namespace editors