#include <engine/core.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/ecs/systems/movementSystem.hpp>
namespace engine
{
    void Core::init()
    {
        // temp sprite editor
        spriteEditor = new editors::SpriteEditor(&sdlInterface, &renderer, &imguiInterface);
        projectEditor = new editors::ProjectEditor(&sdlInterface, &renderer, &imguiInterface);
    }

    void Core::mainLoop()
    {
        graphics::InputEventType eventType = graphics::NO_EVENT;
        while (running)
        {
            timer.tick();
            eventType = handleEvents();

            if (isGamePreviewActive)
            {
                graphics::Renderer gameRenderer(sdlInterface.getWindow(), sdlInterface.getGLContext());
                graphics::Interface gameInterface(WINDOW_WIDTH, WINDOW_HEIGHT);
                while (isGamePreviewActive)
                {
                    float deltaTime = timer.getDeltaTime();
                    if (!runGamePreviewStep(gameRenderer, gameInterface, deltaTime, eventType))
                        break;
                }
            }
            if (isProjectEditorActive)
                projectEditor->run(eventType);
            else
                spriteEditor->run(eventType);
        }
    }

    bool Core::runGamePreviewStep(graphics::Renderer &gameRenderer, graphics::Interface &gameInterface, float deltaTime, graphics::InputEventType gameEventType)
    {
        if (!running || !isGamePreviewActive)
            return false;

        timer.tick();
        gameEventType = handleEvents();

        std::cout << "Game Preview Event: " << gameEventType << std::endl;

        if (gameEventType == graphics::QUIT)
        {
            isGamePreviewActive = false;
            return false;
        }

        // Allow closing preview with F5 pressed in the preview window
        if (gameEventType == graphics::KEY_F5)
        {
            isGamePreviewActive = false;
            std::cout << "Exiting game preview." << std::endl;
            return false;
        }

        update(deltaTime);

        gameRenderer.clear();
        gameRenderer.drawPixelsOverlay(pixels, PIXEL_SIZE);
        gameRenderer.present(gameInterface.getWindow());

        return isGamePreviewActive;
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
            running = false;
            break;
        case graphics::KEY_TAB:
            isProjectEditorActive = !isProjectEditorActive;
            break;
        case graphics::KEY_F5:
            if (!isGamePreviewActive)
                isGamePreviewActive = true;
            break;
        default:
            return eventType;
        }
        return eventType;
    }

    void Core::update(float deltaTime)
    {
        systemManager.update(deltaTime, componentManager);
    }

    void Core::render()
    {
        renderer.clear();
        renderer.drawPixelsOverlay(pixels, PIXEL_SIZE);
        renderer.present(sdlInterface.getWindow());
    }

    void Core::shutdown()
    {
        SDL_Quit();
    }
} // namespace engine