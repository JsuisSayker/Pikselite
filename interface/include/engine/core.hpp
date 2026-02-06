#pragma once

#include <editors/sprite/spriteEditor.hpp>
#include <editors/project/projectEditor.hpp>

#include <engine/time.hpp>
#include <engine/events/events.hpp>
#include <engine/events/eventBus.hpp>
#include <engine/managers/entityManager.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/managers/systemManager.hpp>

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

        void init();

        graphics::InputEventType handleEvents();

        void update(float deltaTime);

        void render();

        void mainLoop();

        // helper: run a single frame (step) of the game preview; returns whether preview continues
        void runGamePreview();
        bool runGamePreviewStep(graphics::Renderer &gameRenderer, graphics::Interface &gameInterface, float deltaTime);
        void shutdown();

        bool copyProjectEditorDataToCore();
    };
} // namespace engine