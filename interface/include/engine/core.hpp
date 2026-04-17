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
        Core(int width, int height)
            : running(true),
              sdlInterface(width, height),
              renderer(sdlInterface.getWindow(), sdlInterface.getGLContext()),
              imguiInterface(sdlInterface.getWindow(), sdlInterface.getGLContext()),
              _pixelSimulation(_chunkGrid) {}

        void run();
        void setCameraPosition(float x, float y) { _camera.setPosition(x, y); }
        void setCameraZoom(float zoom) { _camera.setZoom(zoom); }

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
        b2WorldId _physicsWorld = b2_nullWorldId;

        float accumulator = 0.0f;
        const float fixedDt = 1.0f / 60.0f; // 60 ticks/sec

        // Mapping from Pixel::GameObjectID to ecs::EntityID
        std::unordered_map<Pixel::GameObjectID, ecs::EntityID> _gameObjectToEntity;
        std::string _sceneFilename = "assets/scene.json";

        void init();
        void runPhysicsDemo();
        std::vector<graphics::Pixel> buildSquarePixels(glm::vec2 center, float size, glm::vec3 color) const;
        std::vector<graphics::Pixel> buildRotatedSquarePixels(glm::vec2 center, float size, float rotation, glm::vec3 color) const;
        std::vector<graphics::Pixel> buildRectanglePixels(glm::vec2 center, float width, float height, glm::vec3 color) const;

        graphics::InputEvent handleEvents();

        void update(float deltaTime);

        void render();

        void mainLoop();

        // helper: run a single frame (step) of the game preview; returns whether preview continues
        void runGamePreview();
        void shutdown();

        bool copyProjectEditorDataToCore();

        // Creates an ECS entity for each Pixel::GameObject,
        // create new instance for preview.
        void loadGameObjectsIntoECS();

        // Rebuild and propagate ECS signature from actual component presence.
        void refreshEntitySignature(ecs::EntityID entityId);

        std::vector<graphics::Pixel> buildRenderPixels(ChunkGrid grid) const;

        // save scene and load scene functions for project editor
        void saveScene(const std::string& filename);
        bool loadScene(const std::string& filename);
    };
} // namespace engine