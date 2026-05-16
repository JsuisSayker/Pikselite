#include <SDL2/SDL.h>
#include <algorithm>
#include <editors/project/projectEditor.hpp>

namespace editors
{

    ProjectEditor::ProjectEditor(graphics::Interface* graphicsInterface,
                                 graphics::Renderer* renderer,
                                 graphics::ImguiInterface* imguiInterface,
                                 engine::ComponentManager* componentManager)
        : AbstractEditor(graphicsInterface, renderer, imguiInterface),
          _componentManager(componentManager)
    {
    }

    ProjectEditor::~ProjectEditor() {}

    void ProjectEditor::run(const graphics::InputEvent& event)
    {
        handleEvents(event);

        _renderer->clear();

        _imguiInterface->startFrame();
        _imguiInterface->projectTopBar(_currentProject.name);
        _imguiInterface->gameObjectsBar(_gameObjects, _selectedGameObjectIndex, _currentProject);
        _imguiInterface->setFileExplorerDataOnly(false);
        _imguiInterface->projectNavbar(_currentSpriteFilename, _currentSceneFilename,
                                       _saveSceneRequested, _loadSceneRequested,
                                       _buildGameRequested);

        if (!_currentSpriteFilename.empty())
        {
            if (isTextureFile(_currentSpriteFilename))
            {
                _isPlacingTexture = loadTextureForPlacement(_currentSpriteFilename);
            }
            else
            {
                _isPlacingSprite = loadSpriteForPlacement(_currentSpriteFilename);
            }
            _currentSpriteFilename.clear();
        }

        std::vector<graphics::Pixel> framePixels = buildRenderPixels();
        std::vector<graphics::Pixel> pendingSpritePixels = addPendingSpriteToRenderPixels();
        framePixels.insert(framePixels.end(), pendingSpritePixels.begin(),
                           pendingSpritePixels.end());
        _renderPixels = framePixels; // cache for editing

        _renderer->drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);
        drawGameObjectSprites();
        drawPendingTexturePreview();
        _renderer->drawGrid(_camera, PIXEL_SIZE, {0.7f, 0.7f, 0.7f});

        // Build settings dialog (rendered inside the ImGui frame)
        if (_showBuildDialog)
        {
            bool confirmed = false;
            bool cancelled = false;
            _imguiInterface->buildGameSettingsDialog(_buildDialogSettings, confirmed, cancelled);
            if (confirmed)
            {
                _buildDialogConfirmed = true;
                _showBuildDialog = false;
            }
            if (cancelled)
            {
                _showBuildDialog = false;
            }
        }

        // Build progress modal (rendered inside the ImGui frame)
        if (_showBuildProgress && !_buildProgressDismissed)
        {
            const bool popupOpen = ImGui::IsPopupOpen("Build Progress");
            if (popupOpen)
            {
                float progress = _buildProgressDone ? 1.0f : -1.0f;
                _imguiInterface->showBuildProgressModal(
                    _buildProgressDone ? (_buildProgressSuccess ? "Build complete" : "Build failed")
                                       : "Building...",
                    progress, _buildProgressDone, _buildProgressSuccess,
                    _buildProgressOutput.empty() ? nullptr : _buildProgressOutput.c_str());
            }
            else if (!_buildProgressDone)
            {
                ImGui::OpenPopup("Build Progress");
            }
            if (_buildProgressDone && !ImGui::IsPopupOpen("Build Progress"))
            {
                _buildProgressDismissed = true;
            }
        }

        _imguiInterface->endFrame(_graphicsInterface->getWindow());
        _renderer->present(_graphicsInterface->getWindow());
    }

    void ProjectEditor::setSceneData(const std::vector<graphics::Pixel>& renderPixels,
                                     const std::vector<Pixel::GameObject>& gameObjects,
                                     const ChunkGrid& chunkGrid, uint32_t nextGameObjectId)
    {
        _renderPixels = renderPixels;
        _gameObjects = gameObjects;
        _chunkGrid = chunkGrid;
        gameObjectCounter = nextGameObjectId;
        _pendingSprite = {};
        _isPlacingSprite = false;
        _pendingTexture = {};
        _isPlacingTexture = false;
        _leftMouseDownLastFrame = false;
    }

    void ProjectEditor::handleEvents(const graphics::InputEvent& event)
    {
        switch (event.type)
        {
            case graphics::MOUSE_LEFT_CLICK:
                mouseLeftClick();
                break;
            case graphics::MOUSE_MIDDLE_DRAG:
                panCameraScreenDelta(event.mouseDelta);
                break;
            case graphics::MOUSE_WHEEL:
                if (event.wheelY > 0.0f)
                    zoomAroundMouse(1.1f);
                else if (event.wheelY < 0.0f)
                    zoomAroundMouse(1.0f / 1.1f);
                break;
            case graphics::KEY_ESCAPE:
                _isPlacingTexture = false;
                _pendingTexture = {};
                break;
            case graphics::FILE_DROPPED:
                if (!event.droppedFilePath.empty())
                {
                    const std::string ext =
                        std::filesystem::path(event.droppedFilePath).extension().string();
                    std::string lowerExt = ext;
                    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);

                    if (lowerExt == ".scene")
                    {
                        _currentSceneFilename = event.droppedFilePath;
                        _loadSceneRequested = true;
                    }
                    else if (isTextureFile(event.droppedFilePath))
                    {
                        _isPlacingTexture = loadTextureForPlacement(event.droppedFilePath);
                    }
                }
                break;
            default:
                break;
        }
    }

    void ProjectEditor::setCurrentProject(const projects::Project& project)
    {
        _currentProject = project;
    }

    void ProjectEditor::placePendingSpriteAtWorldInGameObject(glm::vec2 worldPos)
    {
        if (!_pendingSprite.valid)
            return;

        Pixel::GameObject newObject;
        newObject.id = gameObjectCounter++;
        newObject.name = "GameObject_" + std::to_string(newObject.id);
        newObject.sourceDatPath = _pendingSprite.sourceDatPath;

        // Convert world position to grid coords
        int anchorGX = (int)std::floor(worldPos.x / PIXEL_SIZE);
        int anchorGY = (int)std::floor(worldPos.y / PIXEL_SIZE);

        std::vector<Element::Pixel> objectPixels;
        std::vector<Element::Vec2i> objectLocalCoords;

        // Place each cell relative to anchor
        for (const auto& cell : _pendingSprite.cells)
        {
            int targetGX = anchorGX + cell.localGX;
            int targetGY = anchorGY + cell.localGY;

            int cx = toChunk(targetGX);
            int cy = toChunk(targetGY);
            int lx = toLocal(targetGX);
            int ly = toLocal(targetGY);

            Chunk& chunk = _chunkGrid.getOrCreateChunk(cx, cy);
            uint8_t colorIndex = _renderer->generatePixelColorIndex(anchorGX + cell.localGX,
                                                                    anchorGY + cell.localGY);
            ElementDefinition& def = g_elements[cell.type];
            if (cell.type == Element::FIRE)
            {
                uint8_t burnDuration = def.fireParams.burnDuration;
                chunk.set(lx, ly, Element::Pixel{cell.type, false, colorIndex, burnDuration});
                objectPixels.push_back(Element::Pixel{cell.type, false, colorIndex, burnDuration});
            }
            else
            {
                chunk.set(lx, ly, Element::Pixel{cell.type, false, colorIndex});
                objectPixels.push_back(Element::Pixel{cell.type, false, colorIndex});
            }
            objectLocalCoords.push_back({cell.localGX, cell.localGY});
        }

        newObject.pixels = std::move(objectPixels);
        newObject.pixelLocalCoords = std::move(objectLocalCoords);

        ecs::components::Transform transform{};
        transform.enabled = true;
        transform.x = static_cast<float>(anchorGX) * PIXEL_SIZE;
        transform.y = static_cast<float>(anchorGY) * PIXEL_SIZE;
        transform.prevX = transform.x;
        transform.prevY = transform.y;
        transform.rotation = 0.0f;
        transform.scaleX = 1.0f;
        transform.scaleY = 1.0f;
        newObject.addComponent(transform);

        _gameObjects.push_back(std::move(newObject));
        std::cout << "Placed sprite at grid: (" << anchorGX << ", " << anchorGY << ")" << std::endl;
    }

    bool ProjectEditor::isTextureFile(const std::string& path) const
    {
        std::string ext = std::filesystem::path(path).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".webp";
    }

    void ProjectEditor::placeTextureAtWorldInGameObject(glm::vec2 worldPos,
                                                        const std::string& texturePath)
    {
        if (texturePath.empty())
            return;

        Pixel::GameObject newObject;
        newObject.id = gameObjectCounter++;
        newObject.name = "Sprite_" + std::to_string(newObject.id);

        ecs::components::Transform transform{};
        transform.enabled = true;
        transform.x = worldPos.x;
        transform.y = worldPos.y;
        transform.rotation = 0.0f;
        transform.scaleX = 1.0f;
        transform.scaleY = 1.0f;
        transform.prevX = worldPos.x;
        transform.prevY = worldPos.y;

        ecs::components::Sprite sprite{};
        sprite.enabled = true;
        sprite.texturePath = texturePath;
        sprite.width = 640.0f;
        sprite.height = 640.0f;

        newObject.addComponent(transform);
        newObject.addComponent(sprite);

        _gameObjects.push_back(std::move(newObject));
    }

    void ProjectEditor::drawGameObjectSprites()
    {
        for (auto& go : _gameObjects)
        {
            if (!go.isActive)
                continue;

            auto* sprite = go.getComponent<ecs::components::Sprite>();
            auto* transform = go.getComponent<ecs::components::Transform>();
            if (!sprite || !transform || !sprite->enabled)
                continue;

            auto textureIt = _textureCache.find(sprite->texturePath);
            if (textureIt == _textureCache.end())
            {
                const GLuint textureId = _renderer->loadTexture(sprite->texturePath);
                _textureCache[sprite->texturePath] = textureId;
                textureIt = _textureCache.find(sprite->texturePath);
            }

            if (textureIt == _textureCache.end() || textureIt->second == 0)
                continue;

            graphics::Sprite2D sprite2d;
            sprite2d.position = {transform->x, transform->y};
            sprite2d.size = {sprite->width * transform->scaleX, sprite->height * transform->scaleY};
            sprite2d.textureID = textureIt->second;
            _renderer->drawSprite(sprite2d, _camera);
        }
    }

    void ProjectEditor::mouseLeftClick()
    {
        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        if (_isPlacingTexture && _pendingTexture.valid)
        {
            placeTextureAtWorldInGameObject(worldPos, _pendingTexture.texturePath);
            _isPlacingTexture = false;
            _pendingTexture = {};
        }
        else if (_isPlacingSprite)
        {
            placePendingSpriteAtWorldInGameObject(worldPos);
            _isPlacingSprite = false;
            _pendingSprite = {};
        }
        else
        {
            // Handle other left-click interactions (e.g., selecting game objects)
        }
    }

    bool ProjectEditor::loadTextureForPlacement(const std::string& texturePath)
    {
        if (texturePath.empty())
            return false;

        // Check if already in cache, otherwise load it
        auto textureIt = _textureCache.find(texturePath);
        GLuint textureID = 0;
        if (textureIt == _textureCache.end())
        {
            textureID = _renderer->loadTexture(texturePath);
            if (textureID == 0)
                return false;
            _textureCache[texturePath] = textureID;
        }
        else
        {
            textureID = textureIt->second;
        }

        _pendingTexture.valid = true;
        _pendingTexture.texturePath = texturePath;
        _pendingTexture.textureID = textureID;
        _pendingTexture.width = 640.0f;
        _pendingTexture.height = 640.0f;

        return true;
    }

    void ProjectEditor::drawPendingTexturePreview()
    {
        if (!_isPlacingTexture || !_pendingTexture.valid || _pendingTexture.textureID == 0)
            return;

        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        graphics::Sprite2D sprite2d;
        sprite2d.position = worldPos;
        sprite2d.size = {_pendingTexture.width, _pendingTexture.height};
        sprite2d.textureID = _pendingTexture.textureID;

        _renderer->drawSprite(sprite2d, _camera);
    }

    void ProjectEditor::showBuildSettings(const BuildSettings& settings)
    {
        _buildDialogSettings = settings;
        _showBuildDialog = true;
        _buildDialogConfirmed = false;
    }

    bool ProjectEditor::consumeBuildConfirmed(BuildSettings& outSettings)
    {
        if (!_buildDialogConfirmed)
            return false;
        _buildDialogConfirmed = false;
        outSettings = _buildDialogSettings;
        return true;
    }

    void ProjectEditor::setBuildProgress(bool visible, bool done, bool success,
                                         const std::string& output)
    {
        _showBuildProgress = visible;
        _buildProgressDone = done;
        _buildProgressSuccess = success;
        _buildProgressOutput = output;
    }

    bool ProjectEditor::consumeBuildProgressDismissed()
    {
        if (!_buildProgressDismissed)
            return false;
        _buildProgressDismissed = false;
        _showBuildProgress = false;
        return true;
    }

} // namespace editors