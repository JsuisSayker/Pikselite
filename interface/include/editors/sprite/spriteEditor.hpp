/**
 * @file spriteEditor.hpp
 * @brief Declares the SpriteEditor class for managing the sprite editing interface and user interactions.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and interactions.
 */
#pragma once

#include <graphics/renderer/renderer.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/imgui/imguiInterface.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <engine/pixels/simulation/chunk.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <engine/ecs/components/gameObjectComponent.hpp>
#include <limits>
#include <cmath>
#include <unordered_map>

/**
 * @brief The editors namespace contains classes related to editing and managing the project, including the ProjectEditor class which provides methods for handling user input, managing pixel and game object data, and interfacing with the graphics and component systems.
 * The SpriteEditor class is responsible for the sprite editing interface, allowing users to create and modify pixel-based sprites, manage pixel attributes, and save/load sprites.
 */
namespace editors {

    class SpriteEditor {
    public:
#ifdef UNIT_TEST
        bool testRemovePixelAt(glm::vec2 worldPos) { return removePixelAt(worldPos); }
        graphics::Pixel* testGetPixelAt(glm::vec2 worldPos) { return getPixelAt(worldPos); }
        bool testSaveSpriteToFile(const std::string& filename) { return saveSpriteToFile(filename); }
        bool testLoadSpriteFromFile(const std::string& filename) { return loadSpriteFromFile(filename); }
        bool testLoadSpriteForPlacement(const std::string& filename) { return loadSpriteForPlacement(filename); }
        void testPlacePendingSpriteAtWorld(glm::vec2 worldPos) { placePendingSpriteAtWorld(worldPos); }
        glm::vec2 testScreenToWorld(glm::vec2 screenPos) { return screenToWorld(screenPos); }
        void testSetCurrentSpriteFilename(const std::string& filename) { _currentSpriteFilename = filename; }
        void testSetNewSpritePath(const std::string& filename) { _newSpritePath = filename; }
        void testSetShowDefaultPropertiesEditor(bool v) { _showDefaultPropertiesEditor = v; }
        void testSetShowPixelEditor(bool v) { _showPixelEditor = v; }
        void testSetCurrentPixelIndex(int idx) { if (idx >= 0 && idx < (int)_renderPixels.size()) _currentPixel = &_renderPixels[idx]; }
        void testSetEraserActive(bool v) { _isEraserActive = v; }
#endif
        /**
         * @brief Constructs the SpriteEditor, initializing references to the graphics interface, renderer, and ImGui interface. The SpriteEditor will use these interfaces to manage rendering, user input, and the ImGui user interface during the sprite editing process.
         * @param graphicsInterface A pointer to the graphics::Interface for handling window and input events.
         * @param renderer A pointer to the graphics::Renderer for managing rendering of pixels and sprites
         * @param imguiInterface A pointer to the graphics::ImguiInterface for managing the ImGui user interface.
         */
        SpriteEditor(
            graphics::Interface* graphicsInterface,
            graphics::Renderer* renderer,
            graphics::ImguiInterface* imguiInterface);
        /**
         *  @brief Destructs the SpriteEditor, cleaning up any resources if necessary. Note that the SpriteEditor does not own the graphics interface, renderer, or ImGui interface, and assumes they will be valid for the lifetime of the SpriteEditor.
         */
        ~SpriteEditor();

        /**
         * @brief Runs the sprite editor, handling user input and updating the editing state.
         * @param event A graphics::InputEvent representing the latest user input event to be processed by the editor. This method will handle the event and update the editor's state accordingly, including managing pixel placement, pixel attribute editing, and sprite saving/loading requests.
         * Note: This method should be called in the main application loop when the sprite editor is active, to ensure that user input is processed and the editor's state is updated in response to user interactions.
         */
        void run(const graphics::InputEvent& event);

    private:
        // References to the graphics interface, renderer, and ImGui interface for managing rendering, user input, and the ImGui user interface during sprite editing.
        graphics::ImguiInterface* _imguiInterface;
        graphics::Interface* _graphicsInterface;
        graphics::Renderer* _renderer;

        std::vector<graphics::Pixel> _renderPixels;
        graphics::Camera2D _camera;
		ChunkGrid _chunkGrid;

		/**
		 * @brief Handles user input events, updating the editor's state based on the type of event received. This includes managing mouse input for pixel placement and attribute editing, keyboard input for camera movement and zooming, and other input events relevant to the sprite editing process.
		 * @param event A graphics::InputEvent representing the latest user input event to be processed
		 *
		 */
        void handleEvents(const graphics::InputEvent& event);
		void mouseLeftClick();
        void mouseLeftDrag();
        void applyBrushAt(glm::vec2 worldPos, bool erase, bool drag = false);

		/**
		 * @brief Converts screen coordinates to world coordinates based on the current camera view and zoom level. This method is used to determine the corresponding world position for a given screen position, allowing for accurate placement and editing of pixels in the sprite editor based on user input.
		 * @param screenPos A glm::vec2 containing the x and y coordinates in screen
		 * space (origin at top-left corner). The method will convert these coordinates to world coordinates by applying the inverse of the camera's view-projection transformation, taking into account the camera's position and zoom level.
		 * @return A glm::vec2 containing the corresponding x and y coordinates in world space
		 */
        glm::vec2 screenToWorld(glm::vec2 screenPos);
        /**
		 * @brief Retrieves a pointer to the Pixel at the specified world coordinates, if one exists. This method will determine which pixel is located at the given world position by checking the chunk grid and pixel attributes, allowing for selection and editing of pixels in the sprite editor based on user input.
		 * @param worldPos A glm::vec2 containing the x and y coordinates in world
		 * space. The method will calculate the corresponding chunk and local pixel coordinates, and return a pointer to the Pixel at that location if it exists, or nullptr if there is no pixel at the specified coordinates.
		 * @return A pointer to the Pixel at the specified world coordinates, or nullptr if no pixel exists at that location.
		 */
		graphics::Pixel* getPixelAt(glm::vec2 worldPos);
        /**
		 * @brief Removes the pixel at the specified world coordinates, if one exists. This method will determine which pixel is located at the given world position, and if a pixel exists, it will be removed from the chunk grid, pixel attributes, and render pixels vector, effectively erasing the pixel from the sprite being edited.
		 * @param worldPos A glm::vec2 containing the x and y coordinates in world
		 * space. The method will calculate the corresponding chunk and local pixel coordinates, and if a pixel exists at that location, it will be removed from the editor's data structures.
		 * @return A boolean indicating whether a pixel was successfully removed (true) or if no pixel existed at the specified coordinates (false).
		 */
		bool removePixelAt(glm::vec2 worldPos);

		/**
		 * @brief Adds a new pixel at the specified world coordinates with the given default properties. This method will create a new pixel with a unique PixelEntityID, set its attributes based on the provided default properties, and add it to the chunk grid, pixel attributes, and render pixels vector, allowing for the creation of new pixels in the sprite editor based on user input.
		 * @param worldPos A glm::vec2 containing the x and y coordinates in world space where the new pixel should be added. The method will calculate the corresponding chunk and local pixel coordinates, and add the new pixel to the editor's data structures at that location.
		 * @param defaultProperties A Pixel::DefaultPixelProperties struct containing the default attributes for the
		 * new pixel, including color and physical properties. These properties will be applied to the new pixel being added, allowing for consistent default attributes for new pixels created in the sprite editor.
		 * @return A boolean indicating whether the pixel was successfully added (true) or if there
		 */
        void addPixel(glm::vec2 worldPos);

		/**
		 * @brief Handles ImGui user interface interactions, updating the editor's state based on user input in the ImGui interface. This includes managing UI elements for selecting sprites, creating and managing game objects, and triggering scene saving/loading requests.
		 * Note: This method should be called during the editor's run loop after handling input events
		 */
        void imguiHandling();

		/**
		 * @brief Methods for managing sprite placement, including loading a sprite from a file, updating the placement mode based on user input, placing the pending sprite at the specified grid coordinates, and converting screen coordinates to world coordinates for accurate placement. These methods allow for a user-friendly interface for placing sprites in the scene, with visual feedback and the ability to cancel or confirm placements.
		 * @param filename The file path to the sprite image to be loaded for placement. The
		 */
        bool saveSpriteToFile(const std::string& filename);
		/**
		 * @brief Loads a sprite from the specified file path, updating the pending sprite state with the preview pixels and affected cells based on the sprite's dimensions and the current anchor point for placement. This method will read the sprite image file, extract the pixel data, and prepare it for placement in the scene, allowing for a user-friendly interface for placing sprites in the scene with visual feedback.
		 * @param filename The file path to the sprite image to be loaded. The image should
		 * be in a format supported by the graphics::Renderer's texture loading capabilities. If the sprite is successfully loaded, the pending sprite state will be updated with the preview pixels and affected cells based on the sprite's dimensions and the current anchor point for placement.
		 * @return A boolean indicating whether the sprite was successfully loaded (true) or if there
		 */
        bool loadSpriteFromFile(const std::string& filename);

		/**
		 * @brief Updates the placement mode for the pending sprite based on user input, such as mouse movement and clicks. This method will update the preview of the sprite placement, including which cells will be affected and what the new pixels will look like, as the user moves the mouse around the scene. If the user confirms the placement (e.g., by clicking), this method will trigger the actual placement of the sprite in the scene.
		 * Note: This method should be called during the editor's run loop after handling input events, to ensure that the placement preview is updated in response to user interactions.
		 */
		void updatePlacementMode();

        // Imgui state
        graphics::Pixel* _currentPixel = nullptr;

        bool _showPixelEditor = false;
        bool _showDefaultPropertiesEditor = false;
        bool _isEraserActive = false;
        int _selectedTool = 0; // 0 = paint, 1 = eraser
        int _brushSize = 1;

        std::string _currentSpriteFilename;
        std::string _newSpritePath;

		/**
		 * @brief Structs for managing pending sprite placement, including the cells that will be affected by the sprite, the original pixel entity IDs and attributes of those cells, and a preview of the pixels that will be placed. This allows for a preview mode when placing sprites, where the user can see which pixels will be affected and what the new pixels will look like before confirming the placement.
		 */
		struct PendingCell {
			int localGX = 0;
			int localGY = 0;
			Element::ElementType type = Element::ElementType::EMPTY;
		};

		struct PendingSprite {
			bool valid = false;
			std::vector<PendingCell> cells;
		};

        PendingSprite _pendingSprite;
        bool _isPlacingSprite = false;

		/**
		 * @brief Methods for managing sprite placement, including loading a sprite from a file, updating the placement mode based on user input, placing the pending sprite at the specified grid coordinates, and converting screen coordinates to world coordinates for accurate placement. These methods allow for a user-friendly interface for placing sprites in the scene, with visual feedback and the ability to cancel or confirm placements.
		 * @param filename The file path to the sprite image to be loaded for placement. The image should be in a format supported by the graphics::Renderer's texture loading capabilities. If the sprite is successfully loaded, the pending sprite state will be updated with the preview pixels and affected cells based on the sprite's dimensions and the current anchor point for placement.
		 * @return A boolean indicating whether the sprite was successfully loaded (true) or if there was an error loading the sprite (false).
		 */
		bool loadSpriteForPlacement(const std::string& filename);

        /**
		 * @brief Places the pending sprite at the specified world coordinates, updating the chunk grid, pixel attributes, render pixels, and game object associations accordingly. This method will apply the changes to the scene based on the pending sprite's data, including setting the new pixel entity IDs in the chunk grid, updating the render pixels for rendering, and associating the new pixels with the selected game object if applicable. After placement, the pending sprite state will be reset to allow for new placements.
		 * @param worldPos A glm::vec2 containing the x and y coordinates in world space where the anchor point of the sprite should be placed. The method will calculate the corresponding grid coordinates for placement based on the sprite's dimensions and local pixel coordinates, and apply the changes to the editor's data structures accordingly.
		 * @return A boolean indicating whether the sprite was successfully placed (true) or if there
		 */
		void placePendingSpriteAtWorld(glm::vec2 worldPos);

		std::vector<graphics::Pixel> addPendingSpriteToRenderPixels();


		std::vector<graphics::Pixel> buildRenderPixels();
    };
} // namespace editors
