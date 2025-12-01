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
        graphics::Interface sdlInterface;
        graphics::Renderer renderer;
        graphics::ImguiInterface imguiInterface;
        Timer timer;
        events::EventBus eventBus;
        engine::EntityManager entityManager;
        engine::ComponentManager componentManager;
        engine::SystemManager systemManager;

        std::vector<graphics::Pixel> pixels;

        void init();

        graphics::InputEventType handleEvents();

        void update(float deltaTime);

        void render();

        void mainLoop();

        void shutdown();
    };
} // namespace engine