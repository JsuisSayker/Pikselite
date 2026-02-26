#pragma once

#include <editors/sprite/spriteEditor.hpp>
#include <editors/project/projectEditor.hpp>

#include <engine/time.hpp>
#include <engine/events/events.hpp>
#include <engine/events/eventBus.hpp>
#include <engine/managers/entityManager.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/managers/systemManager.hpp>

#include <engine/pixels/simulation/simulation.hpp>
#include <engine/ecs/components/gameObjectComponent.hpp>

#include <vector>
#include <iostream>
#include <SDL2/SDL.h>

namespace engine
{
    class Core
    {
    public:
        Core(int width, int height)
            : running(true),
              sdlInterface(width, height),
              renderer(sdlInterface.getWindow(), sdlInterface.getGLContext()),
              imguiInterface(sdlInterface.getWindow(), sdlInterface.getGLContext()) {}

        void run();

    private:
        // temp
        editors::SpriteEditor *spriteEditor;
        editors::ProjectEditor *projectEditor;
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

        uint32_t pixelIdCounter = 1;
        uint32_t gameObjectCounter = 1;
        
        graphics::Camera2D _camera;
        Pixel::PixelAttributes _pixelAttributes;
        std::vector<Pixel::GameObject> _gameObjects;
        std::vector<graphics::Pixel> _renderPixels;
        Pixel::ChunkGrid _chunkGrid;
        Pixel::PixelSimulation _pixelSimulation;

        // Mapping from Pixel::GameObjectID to ecs::EntityID
        std::unordered_map<Pixel::GameObjectID, ecs::EntityID> _gameObjectToEntity;

        void init();

        graphics::InputEvent handleEvents();

        void update(float deltaTime);

        void render();

        void mainLoop();

        // helper: run a single frame (step) of the game preview; returns whether preview continues
        void runGamePreview();
        void shutdown();

        bool copyProjectEditorDataToCore();

        // Creates an ECS entity for each Pixel::GameObject,
        // attaching Transform and GameObjectLink components.
        void loadGameObjectsIntoECS();
    };
} // namespace engine