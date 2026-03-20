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
#include <engine/pixels/chunk.hpp>
#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>
#include <unordered_map>

/**
 * @brief The editors namespace contains classes related to editing and managing the project, including the ProjectEditor class which provides methods for handling user input, managing pixel and game object data, and interfacing with the graphics and component systems.
 * The ProjectEditor class is responsible for the main editing interface, allowing users to create and modify pixel-based scenes, manage game objects, place sprites, and save/load scenes.
 */
namespace editors {
    class ProjectEditor {
    public:
#ifdef UNIT_TEST
        bool testLoadSpriteForPlacement(const std::string& filename) { return loadSpriteForPlacement(filename); }
        void testPlacePendingSpriteAtGrid(int anchorGX, int anchorGY) { placePendingSpriteAtGrid(anchorGX, anchorGY); }
#endif
        ProjectEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface);
        /**
         * @brief Constructs the ProjectEditor, initializing references to the graphics interface, renderer, ImGui interface, and component manager. The ProjectEditor will use these interfaces to manage rendering, user input, and game object components during the editing process.
         * @param graphicsInterface A pointer to the graphics::Interface for handling window and input events.
         * @param renderer A pointer to the graphics::Renderer for managing rendering of pixels and sprites.
         * @param imguiInterface A pointer to the graphics::ImguiInterface for managing the ImGui user interface.
         * @param componentManager A pointer to the engine::ComponentManager for managing game object components.
         */
        ProjectEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface, engine::ComponentManager* componentManager);
        /**
         * @brief Destructs the ProjectEditor, cleaning up any resources if necessary. Note that the ProjectEditor does not own the graphics interface, renderer, ImGui interface, or component manager, and assumes they will be valid for the lifetime of the ProjectEditor.
         */
        ~ProjectEditor();

        /**
         * @brief Runs the project editor, handling user input and updating the editing state.
         * @param event A graphics::InputEvent representing the latest user input event to be processed by the editor. This method will handle the event and update the editor's state accordingly, including managing pixel placement, game object selection, sprite placement, and scene saving/loading requests.
         */
        void run(const graphics::InputEvent& event);

        // Accessor methods for the current state of the editor, including the pixels to be rendered, game objects, pixel attributes, chunk grid, and counters for pixel and game object IDs.
        std::vector<graphics::Pixel> getPixels() const { return _renderPixels; }
        std::vector<Pixel::GameObject> getGameObjects() const { return _gameObjects; }
        Pixel::PixelAttributes getPixelAttributes() const { return _pixelAttributes; }
        Pixel::ChunkGrid getChunkGrid() const { return _chunkGrid; }
        uint32_t getPixelIdCounter() const { return pixelIdCounter; }
        uint32_t getGameObjectCounter() const { return gameObjectCounter; }

        // Methods for consuming save and load scene requests, which will return whether a request was made and reset the request state. These methods can be called by the main application loop to determine if the user has requested to save or load a scene, and to trigger the appropriate actions in response.
        bool consumeSaveSceneRequest() {
            const bool requested = _saveSceneRequested;
            _saveSceneRequested = false;
            return requested;
        }
        bool consumeLoadSceneRequest() {
            const bool requested = _loadSceneRequested;
            _loadSceneRequested = false;
            return requested;
        }

        /**
         * @brief Retrieves the filename of the currently selected sprite for placement, if any. This method can be used by the main application loop or other parts of the editor to determine which sprite is currently selected for placement in the scene.
         * @return A std::string containing the filename of the currently selected sprite, or an
         */
        void setSceneData(const std::vector<graphics::Pixel>& renderPixels,
                          const std::vector<Pixel::GameObject>& gameObjects,
                          const Pixel::PixelAttributes& pixelAttributes,
                          const Pixel::ChunkGrid& chunkGrid,
                          uint32_t nextPixelId,
                          uint32_t nextGameObjectId);

    private:
        // References to the graphics interface, renderer, ImGui interface, and component manager for managing rendering, user input, and game object components.
        graphics::ImguiInterface* _imguiInterface;
        graphics::Interface* _graphicsInterface;
        graphics::Renderer* _renderer;
        engine::ComponentManager* _componentManager;

        // Camera for managing the view and zoom level during editing
        graphics::Camera2D _camera;

        // Counters for generating unique IDs for pixels and game objects
        uint32_t pixelIdCounter = 1;
        uint32_t gameObjectCounter = 1;

        // Data structures for managing pixel attributes, game objects, render pixels, and the chunk grid during editing
        Pixel::PixelAttributes _pixelAttributes;

        // The list of game objects in the scene, each containing an ID, name, active state, and list of associated pixel entity IDs. This allows for grouping pixels into game objects and managing their properties and interactions as a unit.
        std::vector<Pixel::GameObject> _gameObjects;
        std::vector<graphics::Pixel> _renderPixels;
        Pixel::ChunkGrid _chunkGrid;

        // State for managing pending sprite placement, including the cells affected, original attributes, and preview pixels.
        Pixel::PendingSprite _pendingSprite;
        bool _isPlacingSprite = false;

        // State for tracking mouse input and scene saving/loading requests.
        bool _leftMouseDownLastFrame = false;
        bool _saveSceneRequested = false;
        bool _loadSceneRequested = false;
        std::string _currentSpriteFilename;
        int _selectedGameObjectIndex = -1;

        /**
         * @brief Handles user input events, updating the editor's state based on the type of event received. This includes managing mouse input for pixel placement and game object selection, keyboard input for camera movement and zooming, and other input events relevant to the editing process.
         * @param event A graphics::InputEvent representing the latest user input event to be processed
         * by the editor. The method will update the editor's state accordingly, including managing pixel placement, game object selection, sprite placement, and scene saving/loading requests based on the type and details of the event.
         */
        void handleEvents(const graphics::InputEvent& event);

        /**
         * @brief Handles ImGui user interface interactions, updating the editor's state based on user input in the ImGui interface. This includes managing UI elements for selecting sprites, creating and managing game objects, and triggering scene saving/loading requests.
          * Note: This method should be called during the editor's run loop after handling input events, to ensure that the ImGui interface is updated and responsive to user interactions.
          */
        void imguiHandling();

        /**
         * @brief Methods for managing sprite placement, including loading a sprite from a file, updating the placement mode based on user input, placing the pending sprite at the specified grid coordinates, and converting screen coordinates to world coordinates for accurate placement. These methods allow for a user-friendly interface for placing sprites in the scene, with visual feedback and the ability to cancel or confirm placements.
         * @param filename The file path to the sprite image to be loaded for placement. The image should be in a format supported by the graphics::Renderer's texture loading capabilities. If the sprite is successfully loaded, the pending sprite state will be updated with the preview pixels and affected cells based on the sprite's dimensions and the current anchor point for placement.
         */
        bool loadSpriteForPlacement(const std::string& filename);
        /**
         * @brief Updates the placement mode for the pending sprite based on user input, such as mouse movement and clicks. This method will update the preview of the sprite placement, including which cells will be affected and what the new pixels will look like, as the user moves the mouse around the scene. If the user confirms the placement (e.g., by clicking), this method will trigger the actual placement of the sprite in the scene.
         * Note: This method should be called during the editor's run loop after handling input events
         */
        void updatePlacementMode();
        /**
         * @brief Places the pending sprite at the specified grid coordinates, updating the chunk grid, pixel attributes, render pixels, and game object associations accordingly. This method will apply the changes to the scene based on the pending sprite's data, including setting the new pixel entity IDs in the chunk grid, updating the render pixels for rendering, and associating the new pixels with the selected game object if applicable. After placement, the pending sprite state will be reset to allow for new placements.
         * @param anchorGX The grid x-coordinate of the anchor point for the sprite placement. This coordinate will be used as the reference point for placing the sprite's pixels in the world, based on their local coordinates in the sprite image.
         * @param anchorGY The grid y-coordinate of the anchor point for the sprite placement. This coordinate will be used as the reference point for placing the sprite's pixels in the world, based on their local coordinates in the sprite image.
         */
        void placePendingSpriteAtGrid(int anchorGX, int anchorGY);
        /**
         * @brief Converts screen coordinates (e.g., from mouse input) to world coordinates based on the camera's position and zoom level, allowing for accurate placement of pixels and sprites in the scene. This method will take into account the camera's transformations to ensure that the coordinates returned correspond to the correct location in the world space, which is essential for placing pixels and sprites accurately based on user input.
         * @param screenPos A glm::vec2 containing the x and y coordinates in screen space (origin at top-left corner). This is typically obtained from mouse input events.
         * @return A glm::vec2 containing the corresponding x and y coordinates in world space, taking into account the camera's position and zoom level. These coordinates can be used for placing pixels and sprites in the scene based on user input.
         */
        glm::vec2 screenToWorld(const glm::vec2& screenPos) const;
    };
} // namespace editors