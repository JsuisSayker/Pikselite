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
        graphics::InputEvent event;
        while (running)
        {
            timer.tick();
            event = handleEvents();

            if (isGamePreviewActive)
            {
                SDL_Window *gameWindow = SDL_CreateWindow(
                    "Game Preview",
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    WINDOW_WIDTH, WINDOW_HEIGHT,
                    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

                SDL_GL_MakeCurrent(gameWindow, sdlInterface.getGLContext());

                while (isGamePreviewActive && running)
                {
                    timer.tick();
                    float deltaTime = timer.getDeltaTime();
                    graphics::InputEvent gameEvent = handleEvents();

                    if (gameEvent.type == graphics::WINDOW_CLOSE)
                    {
                        uint32_t gameWindowID = SDL_GetWindowID(gameWindow);
                        uint32_t mainWindowID = sdlInterface.getWindowID();

                        if (gameEvent.windowID == gameWindowID)
                        {
                            isGamePreviewActive = false;
                            break;
                        }
                        if (gameEvent.windowID == mainWindowID)
                        {
                            running = false;
                            break;
                        }
                    }

                    if (gameEvent.type == graphics::KEY_F5)
                    {
                        isGamePreviewActive = false;
                        break;
                    }

                    update(deltaTime);

                    renderer.clear();
                    renderer.drawPixelsOverlay(pixels, PIXEL_SIZE);
                    renderer.present(gameWindow);
                }

                SDL_DestroyWindow(gameWindow);
                SDL_GL_MakeCurrent(sdlInterface.getWindow(), sdlInterface.getGLContext());
                int w, h;
                SDL_GetWindowSize(sdlInterface.getWindow(), &w, &h);
                glViewport(0, 0, w, h);
            }

            if (isProjectEditorActive)
            {
                projectEditor->run(event);
            }
            else
            {
                spriteEditor->run(event);
            }
        }
    }

    bool Core::runGamePreviewStep(graphics::Renderer &gameRenderer, graphics::Interface &gameInterface, float deltaTime, graphics::InputEvent gameEvent)
    {
        if (!running || !isGamePreviewActive)
            return false;

        timer.tick();
        gameEvent = handleEvents();

        if (gameEvent.type == graphics::WINDOW_CLOSE)
        {
            uint32_t gameWindowID = gameInterface.getWindowID();

            if (gameEvent.windowID == gameWindowID)
            {
                isGamePreviewActive = false;
                return false;
            }
            uint32_t mainWindowID = sdlInterface.getWindowID();
            if (gameEvent.windowID == mainWindowID)
            {
                running = false;
                isGamePreviewActive = false;
                return false;
            }
        }

        if (gameEvent.type == graphics::KEY_F5)
        {
            isGamePreviewActive = false;
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

    graphics::InputEvent Core::handleEvents()
    {
        graphics::InputEvent event = sdlInterface.pollEvent();

        switch (event.type)
        {
        case graphics::QUIT:
            running = false;
            break;
        case graphics::WINDOW_CLOSE:
        {
            uint32_t mainWindowID = sdlInterface.getWindowID();
            if (event.windowID == mainWindowID)
            {
                running = false;
            }
            break;
        }
        case graphics::KEY_TAB:
            isProjectEditorActive = !isProjectEditorActive;
            break;
        case graphics::KEY_F5:
            if (!isGamePreviewActive)
                isGamePreviewActive = true;
            break;
        default:
            break;
        }
        return event;
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