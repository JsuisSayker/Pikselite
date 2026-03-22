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
#include <editors/aEditor.hpp>

/**
 * @brief The editors namespace contains classes related to editing and managing the project, including the ProjectEditor class which provides methods for handling user input, managing pixel and game object data, and interfacing with the graphics and component systems.
 * The SpriteEditor class is responsible for the sprite editing interface, allowing users to create and modify pixel-based sprites, manage pixel attributes, and save/load sprites.
 */
namespace editors {

class SpriteEditor : public AbstractEditor {
public:
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
    void run(const graphics::InputEvent& event) override;

private:
    graphics::Pixel* _currentPixel = nullptr;
    bool _showPixelEditor = false;
    bool _showDefaultPropertiesEditor = false;
    bool _isEraserActive = false;
    int _selectedTool = 0;
    int _brushSize = 1;
    std::string _newSpritePath;
	Element::ElementType _currentElementType = Element::ElementType::SAND;

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
		 * @brief Updates the placement mode for the pending sprite based on user input, such as mouse movement and clicks. This method will update the preview of the sprite placement, including which cells will be affected and what the new pixels will look like, as the user moves the mouse around the scene. If the user confirms the placement (e.g., by clicking), this method will trigger the actual placement of the sprite in the scene.
		 * Note: This method should be called during the editor's run loop after handling input events, to ensure that the placement preview is updated in response to user interactions.
		 */
		void updatePlacementMode();

        void placePendingSpriteAtWorld(glm::vec2 worldPos);
        Element::ElementType getElementTypeAt(glm::vec2 worldPos);
};

} // namespace editors
