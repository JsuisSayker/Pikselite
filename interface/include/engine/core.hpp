#pragma once

#include <editors/sprite/spriteEditor.hpp>
#include <editors/project/projectEditor.hpp>

#include <engine/time.hpp>
#include <engine/events/events.hpp>
#include <engine/events/eventBus.hpp>
#include <engine/managers/entityManager.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/managers/systemManager.hpp>
#include <engine/physics/boxWorld.hpp>

#include <engine/ecs/components/spriteComponent.hpp>

#include <engine/pixels/simulation/simulation.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <box2d/box2d.h>
#include <vector>
#include <iostream>
#include <string>
#include <SDL2/SDL.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace engine
{
    class Core
    {
    public:
                /**
                 * @brief Creates the engine core and initializes rendering/editor interfaces.
                 * @param width Window width in pixels.
                 * @param height Window height in pixels.
                 * @return Constructs `Core`.
                 */
        Core(int width, int height)
            : running(true),
              sdlInterface(width, height),
              renderer(sdlInterface.getWindow(), sdlInterface.getGLContext()),
              imguiInterface(sdlInterface.getWindow(), sdlInterface.getGLContext()),
              _pixelSimulation(_chunkGrid) {}

                /**
                 * @brief Runs initialization, main loop, and shutdown.
                 * @return void
                 */
        void run();

                /**
                 * @brief Sets camera world position.
                 * @param x World X position.
                 * @param y World Y position.
                 * @return void
                 */
        void setCameraPosition(float x, float y) { _camera.setPosition(x, y); }

                /**
                 * @brief Sets camera zoom factor.
                 * @param zoom Zoom multiplier.
                 * @return void
                 */
        void setCameraZoom(float zoom) { _camera.setZoom(zoom); }

        /**
         * @brief Draws sprites with layer strictly below the given value.
         * @param layer Upper bound (exclusive).
         */
        void drawSpritesBelowLayer(int layer);

        /**
         * @brief Draws sprites with layer strictly above the given value.
         * @param layer Lower bound (exclusive).
         */
        void drawSpritesAboveLayer(int layer);

    private:
        editors::SpriteEditor *spriteEditor = nullptr;
        editors::ProjectEditor *projectEditor = nullptr;

        
        bool isProjectEditorActive = false;
        
        bool running;
        bool isGamePreviewActive = false;
        graphics::Interface sdlInterface;
        graphics::Renderer renderer;
        graphics::ImguiInterface imguiInterface;
        Timer timer;
        events::EventBus eventBus;
        engine::EntityManager entityManager;
        engine::ComponentManager componentManager;
        engine::SystemManager systemManager;
        
        uint32_t gameObjectCounter = 1;
        
        graphics::Camera2D _camera;
        physics::BoxWorld _boxWorld;
        b2BodyId _cubeBody = b2_nullBodyId;
        b2BodyId _groundBody = b2_nullBodyId;
        std::vector<Pixel::GameObject> _gameObjects;
        std::vector<graphics::Pixel> _renderPixels;
        Simulation _pixelSimulation;
        ChunkGrid _chunkGrid;

        float accumulator = 0.0f;
        const float fixedDt = 1.0f / 60.0f; // 60 ticks/sec

        // Mapping from Pixel::GameObjectID to ecs::EntityID
        std::unordered_map<Pixel::GameObjectID, ecs::EntityID> _gameObjectToEntity;
        std::unordered_map<Pixel::GameObjectID, std::vector<Element::Vec2i>> _gameObjectOccupiedCells;
        std::string _sceneFilename = "assets/scene.json";

        /**
         * @brief Initializes subsystems, ECS registration, and editors.
         * @return void
         */
        void init();

        /**
         * @brief Runs internal physics demo logic.
         * @return void
         */
        void runPhysicsDemo();

        /**
         * @brief Generates a filled square pixel set.
         * @param center Shape center in world units.
         * @param size Square side length in world units.
         * @param color RGB color in normalized range.
         * @return Pixel list for rendering.
         */
        std::vector<graphics::Pixel> buildSquarePixels(glm::vec2 center, float size, glm::vec3 color) const;

        /**
         * @brief Generates a rotated filled square pixel set.
         * @param center Shape center in world units.
         * @param size Square side length in world units.
         * @param rotation Rotation in radians.
         * @param color RGB color in normalized range.
         * @return Pixel list for rendering.
         */
        std::vector<graphics::Pixel> buildRotatedSquarePixels(glm::vec2 center, float size, float rotation, glm::vec3 color) const;

        /**
         * @brief Generates a filled rectangle pixel set.
         * @param center Shape center in world units.
         * @param width Rectangle width in world units.
         * @param height Rectangle height in world units.
         * @param color RGB color in normalized range.
         * @return Pixel list for rendering.
         */
        std::vector<graphics::Pixel> buildRectanglePixels(glm::vec2 center, float width, float height, glm::vec3 color) const;

        /**
         * @brief Polls and processes one input event.
         * @return The processed input event.
         */
        graphics::InputEvent handleEvents();

        /**
         * @brief Updates simulation and ECS for one frame.
         * @param deltaTime Frame delta time in seconds.
         * @return void
         */
        void update(float deltaTime);

        /**
         * @brief Renders one frame to the main window.
         * @return void
         */
        void render();

        /**
         * @brief Runs the main application loop.
         * @return void
         */
        void mainLoop();

        // helper: run a single frame (step) of the game preview; returns whether preview continues
        /**
         * @brief Runs game preview window loop.
         * @return void
         */
        void runGamePreview();

        /**
         * @brief Shuts down subsystems and saves scene state.
         * @return void
         */
        void shutdown();

        /**
         * @brief Copies project editor data into core runtime state.
         * @return `true` if copy happened, otherwise `false`.
         */
        bool copyProjectEditorDataToCore();

        // Creates an ECS entity for each Pixel::GameObject,
        // create new instance for preview.
        /**
         * @brief Rebuilds runtime ECS entities from stored game objects.
         * @return void
         */
        void loadGameObjectsIntoECS();

        /**
         * @brief Synchronizes physics-driven game object pixels back into the grid.
         * @return void
         */
        void syncGameObjectPixelsFromPhysics();

        // Rebuild and propagate ECS signature from actual component presence.
        /**
         * @brief Refreshes one entity signature and system membership.
         * @param entityId Entity to refresh.
         * @return void
         */
        void refreshEntitySignature(ecs::EntityID entityId);

        /**
         * @brief Converts simulation grid contents to renderable pixels.
         * @param grid Grid snapshot to convert.
         * @return Render pixel list.
         */
        std::vector<graphics::Pixel> buildRenderPixels(ChunkGrid grid) const;

        // save scene and load scene functions for project editor
        /**
         * @brief Saves current scene to disk.
         * @param filename Scene file path.
         * @return void
         */
        void saveScene(const std::string& filename);

        /**
         * @brief Loads a scene from disk.
         * @param filename Scene file path.
         * @return `true` on successful load, otherwise `false`.
         */
        bool loadScene(const std::string& filename);
    };
} // namespace engine