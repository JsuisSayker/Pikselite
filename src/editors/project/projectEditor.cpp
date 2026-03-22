#include <editors/project/projectEditor.hpp>
#include <SDL2/SDL.h>

namespace editors {

    ProjectEditor::ProjectEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface, engine::ComponentManager* componentManager)
        : _graphicsInterface(graphicsInterface), _renderer(renderer), _imguiInterface(imguiInterface), _componentManager(componentManager) {}

    ProjectEditor::~ProjectEditor() {}

    void ProjectEditor::run(const graphics::InputEvent& event) {
        handleEvents(event);

        _renderer->clear();

        _imguiInterface->startFrame();
        _imguiInterface->projectTopBarEmpty();
        _imguiInterface->projectNavbar(_currentSpriteFilename, _saveSceneRequested, _loadSceneRequested);

        if (!_currentSpriteFilename.empty()) {
            _isPlacingSprite = loadSpriteForPlacement(_currentSpriteFilename);
            _currentSpriteFilename.clear();
        }

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

    void ProjectEditor::setSceneData(const std::vector<graphics::Pixel>& renderPixels,
                                     const std::vector<Pixel::GameObject>& gameObjects,
                                     const ChunkGrid& chunkGrid,
                                     uint32_t nextGameObjectId) {
        _renderPixels = renderPixels;
        _gameObjects = gameObjects;
        _chunkGrid = chunkGrid;
        gameObjectCounter = nextGameObjectId;
        _pendingSprite = {};
        _isPlacingSprite = false;
        _leftMouseDownLastFrame = false;
    }

    void ProjectEditor::handleEvents(const graphics::InputEvent& event) {
        switch (event.type) {
        case graphics::KEY_W:
            _camera.move(glm::vec2(0.0f, -10.0f));
            break;
        case graphics::MOUSE_LEFT_CLICK:
            mouseLeftClick();
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
            loadSpriteForPlacement("assets/gaz.dat");
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

    bool ProjectEditor::loadSpriteForPlacement(const std::string& filename) {
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

    void ProjectEditor::placePendingSpriteAtWorld(glm::vec2 worldPos) {
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

    std::vector<graphics::Pixel> ProjectEditor::addPendingSpriteToRenderPixels() {
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

    std::vector<graphics::Pixel> ProjectEditor::buildRenderPixels()
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

    void ProjectEditor::imguiHandling()
    {
        _imguiInterface->gameObjectsBar(_gameObjects, _selectedGameObjectIndex, _componentManager);
    }

    void ProjectEditor::mouseLeftClick() {
        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        if (_isPlacingSprite) {
            placePendingSpriteAtWorld(worldPos);
            _isPlacingSprite = false;
            _pendingSprite = {};
        } else {
            // Handle other left-click interactions (e.g., selecting game objects)
        }
    }

} // namespace editors