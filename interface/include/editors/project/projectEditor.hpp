/**
 * @file projectEditor.hpp
 * @brief Declares the ProjectEditor class for managing the project editing interface, including pixel and game object management, sprite placement, and scene saving/loading.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and interactions.
 */
#pragma once

#include <graphics/renderer/renderer.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/imgui/imguiInterface.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <engine/pixels/simulation/chunk.hpp>
#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>
#include <unordered_map>
#include <filesystem>
#include <editors/aEditor.hpp>

#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <projects.hpp>

/**
 * @brief The editors namespace contains classes related to editing and managing the project, including the ProjectEditor class which provides methods for handling user input, managing pixel and game object data, and interfacing with the graphics and component systems.
 * The ProjectEditor class is responsible for the main editing interface, allowing users to create and modify pixel-based scenes, manage game objects, place sprites, and save/load scenes.
 */
namespace editors
{
    class ProjectEditor : public AbstractEditor
    {
    public:
#ifdef UNIT_TEST
        bool testLoadSpriteForPlacement(const std::string &filename) { return loadSpriteForPlacement(filename); }
#endif
        ProjectEditor(graphics::Interface *graphicsInterface, graphics::Renderer *renderer, graphics::ImguiInterface *imguiInterface);
        /**
         * @brief Constructs the ProjectEditor, initializing references to the graphics interface, renderer, ImGui interface, and component manager. The ProjectEditor will use these interfaces to manage rendering, user input, and game object components during the editing process.
         * @param graphicsInterface A pointer to the graphics::Interface for handling window and input events.
         * @param renderer A pointer to the graphics::Renderer for managing rendering of pixels and sprites.
         * @param imguiInterface A pointer to the graphics::ImguiInterface for managing the ImGui user interface.
         * @param componentManager A pointer to the engine::ComponentManager for managing game object components.
         */
        ProjectEditor(graphics::Interface *graphicsInterface, graphics::Renderer *renderer, graphics::ImguiInterface *imguiInterface, engine::ComponentManager *componentManager);
        /**
         * @brief Destructs the ProjectEditor, cleaning up any resources if necessary. Note that the ProjectEditor does not own the graphics interface, renderer, ImGui interface, or component manager, and assumes they will be valid for the lifetime of the ProjectEditor.
         */
        ~ProjectEditor();

        /**
         * @brief Runs the project editor, handling user input and updating the editing state.
         * @param event A graphics::InputEvent representing the latest user input event to be processed by the editor. This method will handle the event and update the editor's state accordingly, including managing pixel placement, game object selection, sprite placement, and scene saving/loading requests.
         */
        void run(const graphics::InputEvent &event) override;

        // Accessor methods for the current state of the editor, including the pixels to be rendered, game objects, pixel attributes, chunk grid, and counters for pixel and game object IDs.
        std::vector<graphics::Pixel> getPixels() const { return _renderPixels; }
        std::vector<Pixel::GameObject> getGameObjects() const { return _gameObjects; }
        ChunkGrid getChunkGrid() const { return _chunkGrid; }
        uint32_t getGameObjectCounter() const { return gameObjectCounter; }

        // Methods for consuming save and load scene requests, which will return whether a request was made and reset the request state. These methods can be called by the main application loop to determine if the user has requested to save or load a scene, and to trigger the appropriate actions in response.
        bool consumeSaveSceneRequest()
        {
            const bool requested = _saveSceneRequested;
            _saveSceneRequested = false;
            return requested;
        }
        bool consumeLoadSceneRequest()
        {
            const bool requested = _loadSceneRequested;
            _loadSceneRequested = false;
            return requested;
        }

        /**
         * @brief Retrieves the filename of the currently selected sprite for placement, if any. This method can be used by the main application loop or other parts of the editor to determine which sprite is currently selected for placement in the scene.
         * @return A std::string containing the filename of the currently selected sprite, or an
         */
        void setSceneData(const std::vector<graphics::Pixel> &renderPixels,
                          const std::vector<Pixel::GameObject> &gameObjects,
                          const ChunkGrid &chunkGrid,
                          uint32_t nextGameObjectId);
        
        
        void setCurrentProject(const projects::Project& project);

    private:
        struct PendingTexture {
            bool valid = false;
            std::string texturePath;
            GLuint textureID = 0;
            float width = 640.0f;
            float height = 640.0f;
        };

        engine::ComponentManager *_componentManager = nullptr;

        uint32_t gameObjectCounter = 1;
        std::vector<Pixel::GameObject> _gameObjects;
        bool _leftMouseDownLastFrame = false;
        bool _saveSceneRequested = false;
        bool _loadSceneRequested = false;
        int _selectedGameObjectIndex = -1;
        std::unordered_map<std::string, GLuint> _textureCache;
        
        bool _isPlacingTexture = false;
        PendingTexture _pendingTexture = {};
        projects::Project _currentProject;

        /**
         * @brief Handles user input events, updating the editor's state based on the type of event received. This includes managing mouse input for pixel placement and game object selection, keyboard input for camera movement and zooming, and other input events relevant to the editing process.
         * @param event A graphics::InputEvent representing the latest user input event to be processed
         * by the editor. The method will update the editor's state accordingly, including managing pixel placement, game object selection, sprite placement, and scene saving/loading requests based on the type and details of the event.
         */
        void handleEvents(const graphics::InputEvent &event);

        /**
         * @brief Places the pending sprite at the specified world coordinates, updating the chunk grid, pixel attributes, render pixels, and game object associations accordingly. This method will apply the changes to the scene based on the pending sprite's data, including setting the new pixel entity IDs in the chunk grid, updating the render pixels for rendering, and associating the new pixels with the selected game object if applicable. After placement, the pending sprite state will be reset to allow for new placements.
         * @param worldPos A glm::vec2 containing the x and y coordinates in world space where the anchor point of the sprite should be placed. The method will calculate the corresponding grid coordinates for placement based on the sprite's dimensions and local pixel coordinates, and apply the changes to the editor's data structures accordingly.
         * @return A boolean indicating whether the sprite was successfully placed (true) or if there
         */
        void placePendingSpriteAtWorldInGameObject(glm::vec2 worldPos);
        bool isTextureFile(const std::string& path) const;
        void placeTextureAtWorldInGameObject(glm::vec2 worldPos, const std::string& texturePath);
        void drawGameObjectSprites();
        
        bool loadTextureForPlacement(const std::string& texturePath);
        void drawPendingTexturePreview();

        void mouseLeftClick();
    };
} // namespace editors