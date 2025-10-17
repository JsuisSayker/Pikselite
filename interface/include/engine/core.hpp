#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include <graphics/interface/interface.hpp>
#include <graphics/renderer/renderer.hpp>
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
              renderer(sdlInterface.getWindow(), sdlInterface.getGLContext()) {}

        void run();

    private:
        bool running;
        graphics::Interface sdlInterface;
        graphics::Renderer renderer;
        Timer timer;
        events::EventBus eventBus;

        std::vector<graphics::Pixel> pixels;

        void init();

        void handleEvents();

        void update(float deltaTime);

        void render();

        void mainLoop();

        void shutdown();
    };
} // namespace engine