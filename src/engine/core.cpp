#include <engine/core.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/ecs/components/gameObjectComponent.hpp>
#include <engine/ecs/systems/movementSystem.hpp>
namespace engine
{
    void Core::init()
    {
        // Register ECS components
        componentManager.registerComponent<ecs::components::Transform>();
        componentManager.registerComponent<ecs::components::Velocity>();
        componentManager.registerComponent<ecs::components::GameObjectLink>();

        // Register ECS systems
        auto &movementSys = systemManager.addSystem<ecs::systems::MovementSystem>();
        ecs::Signature movementSig;
        movementSig.set(componentManager.getComponentType<ecs::components::Transform>());
        movementSig.set(componentManager.getComponentType<ecs::components::Velocity>());
        systemManager.setSignature<ecs::systems::MovementSystem>(movementSig);

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
                runGamePreview();
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

    void Core::runGamePreview()
    {
        SDL_Window *gameWindow = SDL_CreateWindow(
            "Game Preview",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        SDL_GL_MakeCurrent(gameWindow, sdlInterface.getGLContext());

        copyProjectEditorDataToCore();

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
            renderer.drawPixelsWCamera(_renderPixels, _camera, PIXEL_SIZE);
            renderer.present(gameWindow);
        }

        SDL_DestroyWindow(gameWindow);
        SDL_GL_MakeCurrent(sdlInterface.getWindow(), sdlInterface.getGLContext());
        int w, h;
        SDL_GetWindowSize(sdlInterface.getWindow(), &w, &h);
        glViewport(0, 0, w, h);
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
        _pixelSimulation.step(_chunkGrid, _pixelAttributes, _renderPixels, deltaTime);
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
        _chunkGrid = projectEditor->getChunkGrid();

        loadGameObjectsIntoECS();

        return true;
    }

    void Core::loadGameObjectsIntoECS()
    {
        for (auto &[goId, entityId] : _gameObjectToEntity)
        {
            componentManager.entityDestroyed(entityId);
            systemManager.entityDestroyed(entityId);
            entityManager.destroyEntity(ecs::Entity(entityId));
        }
        _gameObjectToEntity.clear();

        for (const auto &go : _gameObjects)
        {
            ecs::Entity entity = entityManager.createEntity();
            ecs::EntityID eid = entity.id;

            ecs::components::Transform transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f};
            componentManager.addComponent<ecs::components::Transform>(eid, transform);

            ecs::components::Velocity velocity{20.0f, 0.0f};
            componentManager.addComponent<ecs::components::Velocity>(eid, velocity);

            ecs::components::GameObjectLink link;
            link.gameObjectId = go.id;
            link.pixelEntities = go.pixelEntities;
            componentManager.addComponent<ecs::components::GameObjectLink>(eid, link);

            ecs::Signature sig;
            sig.set(componentManager.getComponentType<ecs::components::Transform>());
            sig.set(componentManager.getComponentType<ecs::components::Velocity>());
            sig.set(componentManager.getComponentType<ecs::components::GameObjectLink>());
            entityManager.setSignature(eid, sig);
            systemManager.entitySignatureChanged(eid, sig);

            _gameObjectToEntity[go.id] = eid;
        }
    }
} // namespace engine