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

        pixels.push_back({200, 200, 0, 1, 0});
        pixels.push_back({300, 300, 0, 0, 1});

        // temp sprite editor
        spriteEditor = new editors::SpriteEditor(&sdlInterface, &renderer);
    }

    void Core::mainLoop()
    {
        graphics::InputEventType eventType = graphics::NO_EVENT;
        while (running)
        {
            timer.tick();
            eventType = handleEvents();
            update(timer.getDeltaTime());
            spriteEditor->run(eventType);
            //render();
        }
    }

    void Core::run()
    {
        init();
        mainLoop();
        shutdown();
    }

    graphics::InputEventType Core::handleEvents()
    {
        graphics::InputEventType eventType = sdlInterface.pollEvent();

        switch (eventType)
        {
        case graphics::QUIT:
            eventBus.publish(std::make_unique<engine::events::QuitEvent>());
            break;
        default:
            return eventType;
        }
        return eventType;
    }

    void Core::update(float deltaTime) {}

    void Core::render()
    {
        renderer.clear();
        renderer.drawPixels(pixels, PIXEL_SIZE);
        renderer.present(sdlInterface.getWindow());
    }

    void Core::shutdown()
    {
        SDL_Quit();
    }
} // namespace engine