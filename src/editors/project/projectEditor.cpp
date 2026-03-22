#include <editors/project/projectEditor.hpp>
#include <SDL2/SDL.h>

namespace editors {

    ProjectEditor::ProjectEditor(graphics::Interface* graphicsInterface,
                             graphics::Renderer* renderer,
                             graphics::ImguiInterface* imguiInterface,
                             engine::ComponentManager* componentManager)
        : AbstractEditor(graphicsInterface, renderer, imguiInterface),
      _componentManager(componentManager) {}

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
        default:
            break;
        }
    }

    void ProjectEditor::placePendingSpriteAtWorldInGameObject(glm::vec2 worldPos) {
        if (!_pendingSprite.valid) return;

        Pixel::GameObject newObject;
        newObject.id = gameObjectCounter++;
        newObject.name = "GameObject_" + std::to_string(newObject.id);

        // Convert world position to grid coords
        int anchorGX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int anchorGY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        std::vector<Element::Pixel> objectPixels;

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
            objectPixels.push_back(Element::Pixel{cell.type});
        }

        newObject.pixels = std::move(objectPixels);
        _gameObjects.push_back(std::move(newObject));
        std::cout << "Placed sprite at grid: (" << anchorGX << ", " << anchorGY << ")" << std::endl;
    }

    void ProjectEditor::imguiHandling()
    {
        _imguiInterface->gameObjectsBar(_gameObjects, _selectedGameObjectIndex, _componentManager);
    }

    void ProjectEditor::mouseLeftClick() {
        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        if (_isPlacingSprite) {
            placePendingSpriteAtWorldInGameObject(worldPos);
            _isPlacingSprite = false;
            _pendingSprite = {};
        } else {
            // Handle other left-click interactions (e.g., selecting game objects)
        }
    }

} // namespace editors