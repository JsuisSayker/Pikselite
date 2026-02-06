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
                runGamePreview();
            }
            if (isProjectEditorActive)
                projectEditor->run(eventType);
            else
                spriteEditor->run(eventType);
        }
    }

    void Core::runGamePreview()
    {
        graphics::Interface gameInterface(WINDOW_WIDTH, WINDOW_HEIGHT);
        graphics::Renderer gameRenderer(gameInterface.getWindow(), gameInterface.getGLContext());
        
        if (copyProjectEditorDataToCore() == false)
            return;
        while (isGamePreviewActive)
        {
            float deltaTime = timer.getDeltaTime();
            if (!runGamePreviewStep(gameRenderer, gameInterface, deltaTime))
                break;
        }
    }

    bool Core::runGamePreviewStep(graphics::Renderer &gameRenderer, graphics::Interface &gameInterface, float deltaTime)
    {
        if (!running || !isGamePreviewActive)
            return false;

        graphics::InputEventType gameEventType = gameInterface.pollEvent();

        if (gameEventType == graphics::QUIT || gameEventType == graphics::KEY_F5)
        {
            isGamePreviewActive = false;
            return false;
        }
        
        timer.tick();
        update(deltaTime);

        gameRenderer.clear();
        gameRenderer.drawPixelsWCamera(_renderPixels, _camera, PIXEL_SIZE);
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
        renderer.drawPixelsOverlay(_renderPixels, PIXEL_SIZE);
        renderer.present(sdlInterface.getWindow());
    }

    void Core::shutdown()
    {
        SDL_Quit();
    }

    bool Core::copyProjectEditorDataToCore()
    {
        if (!isProjectEditorActive)
            return false;

        _renderPixels = projectEditor->getPixels();
        _gameObjects = projectEditor->getGameObjects();
        _pixelAttributes = projectEditor->getPixelAttributes();
        return true;
    }
} // namespace engine