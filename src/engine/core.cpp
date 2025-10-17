#include <engine/core.hpp>
#include <engine/eventBus.hpp>
#include <engine/events.hpp>

namespace engine
{
    void Core::init()
    {
        // subscribe to quit event
        eventBus.subscribe<engine::events::QuitEvent>([this](const engine::events::QuitEvent &e){
            (void)e;
            running = false;
        });

        pixels.push_back({100, 100, 1, 0, 0, 10});
        pixels.push_back({200, 200, 0, 1, 0, 20});
        pixels.push_back({300, 300, 0, 0, 1, 30});
    }

    void Core::mainLoop()
    {
        while (running)
        {
            timer.tick();
            handleEvents();
            update(timer.getDeltaTime());
            render();
        }
    }

    void Core::run()
    {
        init();
        mainLoop();
        shutdown();
    }

    void Core::handleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                eventBus.publish(std::make_unique<engine::events::QuitEvent>());
            }
        }
    }

    void Core::update(float deltaTime) {}

    void Core::render()
    {
        renderer.clear();
        renderer.drawPixels(pixels);
        renderer.present(sdlInterface.getWindow());
    }

    void Core::shutdown()
    {
        SDL_Quit();
    }
} // namespace engine