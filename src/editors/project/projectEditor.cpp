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

        setProjectPaths(_currentProject.path);

        _renderer->clear();

        _imguiInterface->startFrame();
        _imguiInterface->projectTopBar(_saveSceneRequested, _currentProject.name);

        int deleteRequestIndex = -1;
        _imguiInterface->gameObjectsBar(_gameObjects, _selectedGameObjectIndex, _currentProject,
                                        deleteRequestIndex);
        if (deleteRequestIndex >= 0 && deleteRequestIndex < static_cast<int>(_gameObjects.size()))
        {
            deleteGameObjectAt(deleteRequestIndex);
        }
        _imguiInterface->setFileExplorerDataOnly(false);
        _imguiInterface->projectAssetsNavbar(
            _currentSpriteFilename, _currentSceneFilename, _saveSceneRequested, _loadSceneRequested,
            _buildGameRequested, _projectAssetsPath, _projectScenesPath);

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

        drawSpritesBelowLayer(0);
        _renderer->drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);
        drawSpritesAboveLayer(0);
        drawPendingTexturePreview();
        _renderer->drawGrid(_camera, PIXEL_SIZE, {0.7f, 0.7f, 0.7f});

        // Run an active gizmo drag and overlay the gizmo on top of everything
        // else so axis handles are never occluded by the dragged object.
        updateGizmoDrag();
        drawTranslateGizmo();

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

    void ProjectEditor::setProjectPaths(std::filesystem::path projectPath)
    {
        _projectAssetsPath = projectPath / "Assets";
        _projectScenesPath = (_projectAssetsPath) / "Scenes";
        _currentSceneFilename = (_projectScenesPath / "default.scene").string();
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
                // Also cancel any in-progress gizmo drag and clear the selection
                // so ESC is the universal "back out of what I'm doing" key.
                _gizmoDragHandle = GizmoHandle::None;
                _selectedGameObjectIndex = -1;
                break;
            case graphics::KEY_CTRL_S:
                // Same code path as the top-left "Save" button — skip when an
                // ImGui input field has keyboard focus so Ctrl+S in a textbox
                // doesn't trigger a save.
                if (!ImGui::GetIO().WantCaptureKeyboard)
                {
                    _saveSceneRequested = true;
                }
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
        newObject.pixelCount = newObject.pixels.size();
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

        newObject.pixelCount = 0;

        _gameObjects.push_back(std::move(newObject));
    }

    void ProjectEditor::deleteGameObjectAt(int index)
    {
        if (index < 0 || index >= static_cast<int>(_gameObjects.size()))
            return;

        const Pixel::GameObject& go = _gameObjects[index];

        // Clear the pixels this GameObject occupies in the chunk grid. Pixels are
        // anchored at floor(transform.x / PIXEL_SIZE) — same convention used during
        // placement (see placePendingSpriteAtWorldInGameObject) and load.
        if (const auto* transform = go.getComponent<ecs::components::Transform>())
        {
            const int anchorGX = static_cast<int>(std::floor(transform->x / PIXEL_SIZE));
            const int anchorGY = static_cast<int>(std::floor(transform->y / PIXEL_SIZE));
            const size_t pairCount = std::min(go.pixels.size(), go.pixelLocalCoords.size());

            for (size_t i = 0; i < pairCount; ++i)
            {
                const auto& local = go.pixelLocalCoords[i];
                const int gridX = anchorGX + local.x;
                const int gridY = anchorGY + local.y;
                _chunkGrid.setPixel(gridX, gridY, Element::Pixel{Element::EMPTY, false});
            }
        }

        _gameObjects.erase(_gameObjects.begin() + index);

        // Keep the inspector's selection consistent with the new vector layout.
        if (_selectedGameObjectIndex == index)
            _selectedGameObjectIndex = -1;
        else if (_selectedGameObjectIndex > index)
            _selectedGameObjectIndex--;
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

    void ProjectEditor::drawSpritesBelowLayer(int layer)
    {
        struct SpriteDrawItem
        {
            int layer;
            std::size_t index;
        };

        std::vector<SpriteDrawItem> drawList;
        drawList.reserve(_gameObjects.size());

        for (std::size_t i = 0; i < _gameObjects.size(); ++i)
        {
            auto& go = _gameObjects[i];
            if (!go.isActive)
                continue;

            auto* sprite = go.getComponent<ecs::components::Sprite>();
            auto* transform = go.getComponent<ecs::components::Transform>();
            if (!sprite || !transform || !sprite->enabled)
                continue;

            if (sprite->layer >= layer)
                continue;

            drawList.push_back({sprite->layer, i});
        }

        std::sort(drawList.begin(), drawList.end(),
                  [](const SpriteDrawItem& a, const SpriteDrawItem& b)
                  {
                      if (a.layer != b.layer)
                          return a.layer < b.layer;
                      return a.index < b.index;
                  });

        for (const auto& item : drawList)
        {
            auto& go = _gameObjects[item.index];
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

    void ProjectEditor::drawSpritesAboveLayer(int layer)
    {
        struct SpriteDrawItem
        {
            int layer;
            std::size_t index;
        };

        std::vector<SpriteDrawItem> drawList;
        drawList.reserve(_gameObjects.size());

        for (std::size_t i = 0; i < _gameObjects.size(); ++i)
        {
            auto& go = _gameObjects[i];
            if (!go.isActive)
                continue;

            auto* sprite = go.getComponent<ecs::components::Sprite>();
            auto* transform = go.getComponent<ecs::components::Transform>();
            if (!sprite || !transform || !sprite->enabled)
                continue;

            if (sprite->layer <= layer)
                continue;

            drawList.push_back({sprite->layer, i});
        }

        std::sort(drawList.begin(), drawList.end(),
                  [](const SpriteDrawItem& a, const SpriteDrawItem& b)
                  {
                      if (a.layer != b.layer)
                          return a.layer < b.layer;
                      return a.index < b.index;
                  });

        for (const auto& item : drawList)
        {
            auto& go = _gameObjects[item.index];
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
        // Skip scene-space click handling when the click was meant for an ImGui
        // panel (Hierarchy, Inspector, top/bottom bars). Without this guard a
        // click on the Inspector would also try to place a sprite or hit the gizmo.
        if (ImGui::GetIO().WantCaptureMouse)
            return;

        glm::vec2 mousePos = _graphicsInterface->getMousePosition();
        glm::vec2 worldPos = screenToWorld(mousePos);

        if (_isPlacingTexture && _pendingTexture.valid)
        {
            placeTextureAtWorldInGameObject(worldPos, _pendingTexture.texturePath);
            _isPlacingTexture = false;
            _pendingTexture = {};
            return;
        }
        if (_isPlacingSprite)
        {
            placePendingSpriteAtWorldInGameObject(worldPos);
            _isPlacingSprite = false;
            _pendingSprite = {};
            return;
        }

        // If a GameObject is already selected and the click landed on one of its
        // gizmo handles, start a drag instead of changing the selection.
        if (_selectedGameObjectIndex >= 0 &&
            _selectedGameObjectIndex < static_cast<int>(_gameObjects.size()))
        {
            const GizmoHandle handle = hitTestGizmo(worldPos);
            if (handle != GizmoHandle::None)
            {
                const auto* transform = _gameObjects[_selectedGameObjectIndex]
                                            .getComponent<ecs::components::Transform>();
                if (transform)
                {
                    _gizmoDragHandle = handle;
                    _dragStartMouseWorld = worldPos;
                    _dragStartTransform = {transform->x, transform->y};
                    return;
                }
            }
        }

        // Otherwise: pick the topmost GameObject under the cursor (or deselect
        // by setting -1 when the click hits empty space).
        _selectedGameObjectIndex = pickGameObjectAtWorld(worldPos);
    }

    int ProjectEditor::pickGameObjectAtWorld(const glm::vec2& world) const
    {
        // Iterate in reverse so the topmost (last-drawn) GameObject wins.
        for (int i = static_cast<int>(_gameObjects.size()) - 1; i >= 0; --i)
        {
            const Pixel::GameObject& go = _gameObjects[i];
            const auto* transform = go.getComponent<ecs::components::Transform>();
            if (!transform)
                continue;

            // Sprite test: world-space AABB centered on the transform.
            if (const auto* sprite = go.getComponent<ecs::components::Sprite>())
            {
                if (sprite->enabled)
                {
                    const float halfW = sprite->width * transform->scaleX * 0.5f;
                    const float halfH = sprite->height * transform->scaleY * 0.5f;
                    if (world.x >= transform->x - halfW && world.x <= transform->x + halfW &&
                        world.y >= transform->y - halfH && world.y <= transform->y + halfH)
                    {
                        return i;
                    }
                }
            }

            // Pixel test: convert world → grid, see if any of this object's local
            // coords match (anchor-shifted).
            if (!go.pixelLocalCoords.empty())
            {
                const int gridX = static_cast<int>(std::floor(world.x / PIXEL_SIZE));
                const int gridY = static_cast<int>(std::floor(world.y / PIXEL_SIZE));
                const int anchorGX = static_cast<int>(std::floor(transform->x / PIXEL_SIZE));
                const int anchorGY = static_cast<int>(std::floor(transform->y / PIXEL_SIZE));

                for (const auto& local : go.pixelLocalCoords)
                {
                    if (anchorGX + local.x == gridX && anchorGY + local.y == gridY)
                        return i;
                }
            }
        }
        return -1;
    }

    bool ProjectEditor::isPixelBasedGameObject(int index) const
    {
        if (index < 0 || index >= static_cast<int>(_gameObjects.size()))
            return false;
        return !_gameObjects[index].pixelLocalCoords.empty();
    }

    ProjectEditor::GizmoHandle ProjectEditor::hitTestGizmo(const glm::vec2& world) const
    {
        if (_selectedGameObjectIndex < 0 ||
            _selectedGameObjectIndex >= static_cast<int>(_gameObjects.size()))
            return GizmoHandle::None;

        const auto* transform =
            _gameObjects[_selectedGameObjectIndex].getComponent<ecs::components::Transform>();
        if (!transform)
            return GizmoHandle::None;

        const float tx = transform->x;
        const float ty = transform->y;

        // Hit zones match the rendered geometry. Order matters: center first
        // (smallest), then shafts, then arrowhead boxes (which extend ±2 cells
        // around their axis). The shaft boxes are kept thin so axis-locked drags
        // are easy to grab; the head boxes are bigger so users can click the
        // arrowhead directly without missing.
        const float P = PIXEL_SIZE;

        const float centerHalf = P * 1.5f; // matches the "+" plus a touch of padding
        if (world.x >= tx - centerHalf && world.x <= tx + centerHalf &&
            world.y >= ty - centerHalf && world.y <= ty + centerHalf)
        {
            return GizmoHandle::Center;
        }

        // X shaft: thin band along +X, from just past the center to just before
        // the head. X head: ±2 cells wide, spanning cols 3..5.
        if (world.x >= tx + P * 1.5f && world.x <= tx + P * 2.5f && world.y >= ty - P * 0.5f &&
            world.y <= ty + P * 0.5f)
        {
            return GizmoHandle::AxisX;
        }
        if (world.x >= tx + P * 2.5f && world.x <= tx + P * 5.5f && world.y >= ty - P * 2.5f &&
            world.y <= ty + P * 2.5f)
        {
            return GizmoHandle::AxisX;
        }

        // Y shaft + head, mirrored.
        if (world.x >= tx - P * 0.5f && world.x <= tx + P * 0.5f && world.y >= ty + P * 1.5f &&
            world.y <= ty + P * 2.5f)
        {
            return GizmoHandle::AxisY;
        }
        if (world.x >= tx - P * 2.5f && world.x <= tx + P * 2.5f && world.y >= ty + P * 2.5f &&
            world.y <= ty + P * 5.5f)
        {
            return GizmoHandle::AxisY;
        }
        return GizmoHandle::None;
    }

    void ProjectEditor::moveSelectedGameObjectTo(const glm::vec2& newTransform)
    {
        if (_selectedGameObjectIndex < 0 ||
            _selectedGameObjectIndex >= static_cast<int>(_gameObjects.size()))
            return;

        Pixel::GameObject& go = _gameObjects[_selectedGameObjectIndex];
        auto* transform = go.getComponent<ecs::components::Transform>();
        if (!transform)
            return;

        glm::vec2 target = newTransform;

        const bool pixelBased = !go.pixelLocalCoords.empty();
        if (pixelBased)
        {
            // Snap to the pixel grid so cells stay aligned to PIXEL_SIZE — no
            // half-cell offsets, no torn pixels.
            target.x = std::floor(target.x / PIXEL_SIZE + 0.5f) * PIXEL_SIZE;
            target.y = std::floor(target.y / PIXEL_SIZE + 0.5f) * PIXEL_SIZE;
        }

        if (pixelBased)
        {
            const int oldAnchorGX = static_cast<int>(std::floor(transform->x / PIXEL_SIZE));
            const int oldAnchorGY = static_cast<int>(std::floor(transform->y / PIXEL_SIZE));
            const int newAnchorGX = static_cast<int>(std::floor(target.x / PIXEL_SIZE));
            const int newAnchorGY = static_cast<int>(std::floor(target.y / PIXEL_SIZE));

            if (oldAnchorGX != newAnchorGX || oldAnchorGY != newAnchorGY)
            {
                const size_t pairCount = std::min(go.pixels.size(), go.pixelLocalCoords.size());

                // 1) Clear the cells at the old anchor.
                for (size_t i = 0; i < pairCount; ++i)
                {
                    const auto& local = go.pixelLocalCoords[i];
                    _chunkGrid.setPixel(oldAnchorGX + local.x, oldAnchorGY + local.y,
                                        Element::Pixel{Element::EMPTY, false});
                }

                // 2) Write them at the new anchor, preserving each pixel's stored data.
                for (size_t i = 0; i < pairCount; ++i)
                {
                    const auto& local = go.pixelLocalCoords[i];
                    const auto& src = go.pixels[i];
                    if (src.type == Element::EMPTY)
                        continue;
                    _chunkGrid.setPixel(newAnchorGX + local.x, newAnchorGY + local.y, src);
                }
            }
        }

        transform->prevX = target.x;
        transform->prevY = target.y;
        transform->x = target.x;
        transform->y = target.y;
    }

    void ProjectEditor::updateGizmoDrag()
    {
        if (_gizmoDragHandle == GizmoHandle::None)
            return;

        // Drag ends as soon as the left button is released. Polling is simpler
        // than wiring a button-up event through the input system.
        const Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);
        if (!(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)))
        {
            _gizmoDragHandle = GizmoHandle::None;
            return;
        }

        if (_selectedGameObjectIndex < 0 ||
            _selectedGameObjectIndex >= static_cast<int>(_gameObjects.size()))
        {
            _gizmoDragHandle = GizmoHandle::None;
            return;
        }

        const glm::vec2 mouseScreen = _graphicsInterface->getMousePosition();
        const glm::vec2 mouseWorld = screenToWorld(mouseScreen);
        glm::vec2 delta = mouseWorld - _dragStartMouseWorld;

        // Axis lock — zero out the off-axis component for AxisX / AxisY handles.
        if (_gizmoDragHandle == GizmoHandle::AxisX)
            delta.y = 0.0f;
        else if (_gizmoDragHandle == GizmoHandle::AxisY)
            delta.x = 0.0f;

        moveSelectedGameObjectTo(_dragStartTransform + delta);
    }

    void ProjectEditor::drawTranslateGizmo()
    {
        if (_selectedGameObjectIndex < 0 ||
            _selectedGameObjectIndex >= static_cast<int>(_gameObjects.size()))
            return;

        const auto* transform =
            _gameObjects[_selectedGameObjectIndex].getComponent<ecs::components::Transform>();
        if (!transform)
            return;

        const float tx = transform->x;
        const float ty = transform->y;

        // Two-tone palette: shaft uses the darker shade, arrowhead the brighter one.
        // Reads as a proper arrow at low zoom while keeping the pixel-art look.
        const glm::vec3 RED_SHAFT = {0.78f, 0.15f, 0.15f};
        const glm::vec3 RED_HEAD = {1.00f, 0.35f, 0.35f};
        const glm::vec3 GREEN_SHAFT = {0.15f, 0.72f, 0.20f};
        const glm::vec3 GREEN_HEAD = {0.40f, 1.00f, 0.45f};
        const glm::vec3 YELLOW_CORE = {1.00f, 0.92f, 0.20f};
        const glm::vec3 YELLOW_EDGE = {1.00f, 0.78f, 0.10f};

        // Build the gizmo as colored "pixels" sized to PIXEL_SIZE so it reads as
        // grid-cell-sized arrows at any zoom. Reuses the point-sprite shader.
        std::vector<graphics::Pixel> gizmoPixels;
        gizmoPixels.reserve(32);

        auto pushPx = [&](float gx, float gy, const glm::vec3& color)
        {
            graphics::Pixel p;
            p.position = {tx + gx * PIXEL_SIZE, ty + gy * PIXEL_SIZE};
            p.color = color;
            gizmoPixels.push_back(p);
        };

        // ── Yellow center: small "plus" gives a clear pivot dot without merging
        // into the start of the X / Y shafts.
        pushPx(0.0f, 0.0f, YELLOW_CORE);
        pushPx(0.0f, 1.0f, YELLOW_EDGE);
        pushPx(0.0f, -1.0f, YELLOW_EDGE);
        pushPx(1.0f, 0.0f, YELLOW_EDGE);
        pushPx(-1.0f, 0.0f, YELLOW_EDGE);

        // ── Red X arrow (right). Shaft from x=2..3, then a triangular head
        // 5 tall at col 3, 3 tall at col 4, single tip at col 5.
        //
        //   col: 0  1  2  3  4  5
        //   +2:  .  .  .  .  .  .
        //   +1:  .  .  .  H  .  .
        //    0:  O  c  S  S  H  .
        //   -1:  .  .  .  H  .  .
        //   -2:  .  .  .  .  .  .
        pushPx(2.0f, 0.0f, RED_SHAFT);
        pushPx(3.0f, 0.0f, RED_HEAD);
        pushPx(3.0f, 1.0f, RED_HEAD);
        pushPx(3.0f, -1.0f, RED_HEAD);
        pushPx(4.0f, 0.0f, RED_HEAD);

        // ── Green Y arrow (up). Same layout mirrored 90°.
        pushPx(0.0f, 2.0f, GREEN_SHAFT);
        pushPx(0.0f, 3.0f, GREEN_HEAD);
        pushPx(1.0f, 3.0f, GREEN_HEAD);
        pushPx(-1.0f, 3.0f, GREEN_HEAD);
        pushPx(0.0f, 4.0f, GREEN_HEAD);

        _renderer->drawPixelsWCamera(gizmoPixels, _camera, PIXEL_SIZE);
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