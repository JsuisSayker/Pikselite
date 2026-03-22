#include <engine/core.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/ecs/components/gameObjectComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/systems/movementSystem.hpp>
#include <tracy/Tracy.hpp>

#ifndef TRACY_ENABLE
// output a warning if profiling is disabled
#pragma message("Tracy profiling is disabled. To enable, set PIKSELITE_ENABLE_PROFILING=ON in CMake and rebuild.")
#error "Not set"
#endif
#include <engine/ecs/systems/spriteRenderSystem.hpp>
#include <engine/ecs/systems/scriptSystem.hpp>
#include <fstream>
#include <filesystem>

namespace {
    json pixelToJson(const graphics::Pixel& pixel)
    {
        return {
            {"x", pixel.position.x},
            {"y", pixel.position.y},
            {"r", pixel.color.r},
            {"g", pixel.color.g},
            {"b", pixel.color.b}
        };
    }

    graphics::Pixel pixelFromJson(const json& j)
    {
        return {
            {j.value("x", 0.0f), j.value("y", 0.0f)},
            {j.value("r", 1.0f), j.value("g", 1.0f), j.value("b", 1.0f)}
        };
    }

    json gameObjectToJson(const Pixel::GameObject& go)
    {
        json pixelIds = json::array();
        for (const auto pixelId : go.pixelEntities)
            pixelIds.push_back(pixelId);

        return {
            {"id", go.id},
            {"name", go.name},
            {"pixelEntities", pixelIds}
        };
    }

    Pixel::GameObject gameObjectFromJson(const json& j)
    {
        Pixel::GameObject go;
        go.id = j.value("id", Pixel::NO_SPRITE);
        go.name = j.value("name", std::string("GameObject ") + std::to_string(go.id));

        if (j.contains("pixelEntities") && j["pixelEntities"].is_array()) {
            for (const auto& pixelId : j["pixelEntities"]) {
                go.pixelEntities.push_back(pixelId.get<Pixel::PixelEntityID>());
            }
        }

        return go;
    }
}

namespace engine
{
    void Core::init()
    {
        // Register ECS components
        componentManager.registerComponent<ecs::components::Transform>();
        componentManager.registerComponent<ecs::components::Velocity>();
        componentManager.registerComponent<ecs::components::GameObjectLink>();
        componentManager.registerComponent<ecs::components::Sprite>();

        // Register ECS systems
        auto &movementSys = systemManager.addSystem<ecs::systems::MovementSystem>();
        ecs::Signature movementSig;
        movementSig.set(componentManager.getComponentType<ecs::components::Transform>());
        movementSig.set(componentManager.getComponentType<ecs::components::Velocity>());
        systemManager.setSignature<ecs::systems::MovementSystem>(movementSig);

        // Sprite render system (needs Transform + Sprite)
        auto &spriteRenderSys = systemManager.addSystem<ecs::systems::SpriteRenderSystem>(&renderer, &_camera);
        ecs::Signature spriteSig;
        spriteSig.set(componentManager.getComponentType<ecs::components::Transform>());
        spriteSig.set(componentManager.getComponentType<ecs::components::Sprite>());
        systemManager.setSignature<ecs::systems::SpriteRenderSystem>(spriteSig);

        // Script system (needs Transform + Velocity) — runs Lua scripts
        auto &scriptSys = systemManager.addSystem<ecs::systems::ScriptSystem>();
        ecs::Signature scriptSig;
        scriptSig.set(componentManager.getComponentType<ecs::components::Transform>());
        scriptSig.set(componentManager.getComponentType<ecs::components::Velocity>());
        systemManager.setSignature<ecs::systems::ScriptSystem>(scriptSig);
        scriptSys.init();
        scriptSys.loadScript("scripts/movement.lua");

        spriteEditor = new editors::SpriteEditor(&sdlInterface, &renderer, &imguiInterface);
        projectEditor = new editors::ProjectEditor(&sdlInterface, &renderer, &imguiInterface, &componentManager);
    }

    void Core::mainLoop()
    {
        graphics::InputEvent event;
        while (running)
        {
            ZoneScopedN("Frame");
            timer.tick();
            {
                ZoneScopedN("Input");
                event = handleEvents();
            }

            if (isGamePreviewActive)
            {
                ZoneScopedN("GamePreview");
                runGamePreview();
            }

            if (isProjectEditorActive)
            {
                ZoneScopedN("ProjectEditor");
                projectEditor->run(event);

                if (projectEditor->consumeSaveSceneRequest()) {
                    copyProjectEditorDataToCore();
                    saveScene(_sceneFilename);
                }

                if (projectEditor->consumeLoadSceneRequest()) {
                    if (loadScene(_sceneFilename)) {
                        projectEditor->setSceneData(_renderPixels, _gameObjects, _chunkGrid, gameObjectCounter);
                    }
                }
            }
            else
            {
                ZoneScopedN("SpriteEditor");
                spriteEditor->run(event);
            }
            FrameMark;
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

            // Render all entities that have a SpriteComponent via the ECS system
            auto *spriteSystem = systemManager.getSystem<ecs::systems::SpriteRenderSystem>();
            if (spriteSystem)
                spriteSystem->update(0.0, componentManager);

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
        ZoneScoped;

        {
            ZoneScopedN("PixelSimulation");
        }
        {
            ZoneScopedN("ECS Systems");
            systemManager.update(deltaTime, componentManager);
        }
    }

    void Core::render()
    {
        renderer.clear();
        renderer.drawPixelsOverlay(_renderPixels, PIXEL_SIZE);
        renderer.present(sdlInterface.getWindow());
    }

    void Core::shutdown()
    {
        if (projectEditor) {
            copyProjectEditorDataToCore();
            saveScene(_sceneFilename);
        }
        SDL_Quit();
    }

    bool Core::copyProjectEditorDataToCore()
    {
        if (!isProjectEditorActive)
            return false;

        _renderPixels = projectEditor->getPixels();
        _gameObjects = projectEditor->getGameObjects();
        _chunkGrid = projectEditor->getChunkGrid();
        gameObjectCounter = projectEditor->getGameObjectCounter();

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

            ecs::components::Sprite spriteComp;
            spriteComp.texturePath = "assets/dragon.png";
            componentManager.addComponent<ecs::components::Sprite>(eid, spriteComp);

            ecs::Signature sig;
            sig.set(componentManager.getComponentType<ecs::components::Transform>());
            sig.set(componentManager.getComponentType<ecs::components::Velocity>());
            sig.set(componentManager.getComponentType<ecs::components::GameObjectLink>());
            sig.set(componentManager.getComponentType<ecs::components::Sprite>());

            entityManager.setSignature(eid, sig);
            systemManager.entitySignatureChanged(eid, sig);

            _gameObjectToEntity[go.id] = eid;
        }
    }

    void Core::saveScene(const std::string &filename)
    {
        // TODO
    }

    bool Core::loadScene(const std::string &filename)
    {
        // TODO
        return false;
    }

} // namespace engine