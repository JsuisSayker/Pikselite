#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include <editors/sprite/spriteEditor.hpp>
#include <editors/project/projectEditor.hpp>
#include <engine/time.hpp>
#include <engine/eventBus.hpp>
#include <engine/events.hpp>
#include <iostream>

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
        editors::SpriteEditor* spriteEditor;
        editors::ProjectEditor* projectEditor;
        bool isProjectEditorActive = false;

        bool running;
        graphics::Interface sdlInterface;
        graphics::Renderer renderer;
        graphics::ImguiInterface imguiInterface;
        Timer timer;
        events::EventBus eventBus;

        std::vector<graphics::Pixel> pixels;

        void init();

        graphics::InputEventType handleEvents();

        void update(float deltaTime);

        void render();

        void mainLoop();

        void shutdown();
    };
} // namespace engine