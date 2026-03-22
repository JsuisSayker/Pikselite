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
                        projectEditor->setSceneData(_renderPixels, _gameObjects, _pixelAttributes, _chunkGrid, pixelIdCounter, gameObjectCounter);
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
            _pixelSimulation.step(_chunkGrid, _pixelAttributes, _renderPixels, deltaTime);
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
        _pixelAttributes = projectEditor->getPixelAttributes();
        _chunkGrid = projectEditor->getChunkGrid();
        pixelIdCounter = projectEditor->getPixelIdCounter();
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
        json scene;
        scene["version"] = 1;
        scene["pixelIdCounter"] = pixelIdCounter;
        scene["gameObjectCounter"] = gameObjectCounter;

        scene["renderPixels"] = json::array();
        for (const auto& pixel : _renderPixels) {
            scene["renderPixels"].push_back(pixelToJson(pixel));
        }

        scene["gameObjects"] = json::array();
        for (const auto& gameObject : _gameObjects) {
            scene["gameObjects"].push_back(gameObjectToJson(gameObject));
        }

        scene["pixelAttributes"]["renderIndex"] = json::array();
        for (const auto& [id, index] : _pixelAttributes.renderIndex) {
            scene["pixelAttributes"]["renderIndex"].push_back({
                {"id", id},
                {"index", index}
            });
        }

        scene["pixelAttributes"]["solidAttributes"] = json::array();
        for (const auto& [id, solid] : _pixelAttributes.solidAttributes) {
            (void)solid;
            scene["pixelAttributes"]["solidAttributes"].push_back({{"id", id}});
        }

        scene["pixelAttributes"]["liquidAttributes"] = json::array();
        for (const auto& [id, liquid] : _pixelAttributes.liquidAttributes) {
            scene["pixelAttributes"]["liquidAttributes"].push_back({
                {"id", id},
                {"viscosity", liquid.viscosity},
                {"updateThisFrame", liquid.updateThisFrame}
            });
        }

        scene["pixelAttributes"]["gaseousAttributes"] = json::array();
        for (const auto& [id, gaseous] : _pixelAttributes.gaseousAttributes) {
            scene["pixelAttributes"]["gaseousAttributes"].push_back({
                {"id", id},
                {"density", gaseous.density}
            });
        }

        scene["chunkGrid"] = json::array();
        for (const auto& [coord, chunk] : _chunkGrid.getChunks()) {
            json cells = json::array();
            for (int x = 0; x < Pixel::CHUNKS_SIZE; ++x) {
                for (int y = 0; y < Pixel::CHUNKS_SIZE; ++y) {
                    const auto id = chunk.get(x, y);
                    if (id == Pixel::EMPTY)
                        continue;

                    cells.push_back({
                        {"x", x},
                        {"y", y},
                        {"id", id}
                    });
                }
            }

            scene["chunkGrid"].push_back({
                {"cx", coord.first},
                {"cy", coord.second},
                {"cells", cells}
            });
        }

        std::filesystem::path path(filename);
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream file(filename);
        if (file.is_open()) {
            file << scene.dump(4);
        }
    }

    bool Core::loadScene(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            return false;

        json scene;
        try {
            file >> scene;
        } catch (...) {
            return false;
        }

        _renderPixels.clear();
        _gameObjects.clear();
        _pixelAttributes = {};
        _chunkGrid = Pixel::ChunkGrid();
        _gameObjectToEntity.clear();

        pixelIdCounter = scene.value("pixelIdCounter", 1u);
        gameObjectCounter = scene.value("gameObjectCounter", 1u);

        if (scene.contains("renderPixels") && scene["renderPixels"].is_array()) {
            for (const auto& pixel : scene["renderPixels"]) {
                _renderPixels.push_back(pixelFromJson(pixel));
            }
        }

        if (scene.contains("gameObjects") && scene["gameObjects"].is_array()) {
            for (const auto& gameObject : scene["gameObjects"]) {
                _gameObjects.push_back(gameObjectFromJson(gameObject));
            }
        }

        if (scene.contains("pixelAttributes")) {
            const auto& pixelAttributes = scene["pixelAttributes"];

            if (pixelAttributes.contains("renderIndex")) {
                for (const auto& entry : pixelAttributes["renderIndex"]) {
                    _pixelAttributes.renderIndex[entry.at("id").get<Pixel::PixelEntityID>()] = entry.at("index").get<int>();
                }
            }

            if (pixelAttributes.contains("solidAttributes")) {
                for (const auto& entry : pixelAttributes["solidAttributes"]) {
                    _pixelAttributes.solidAttributes[entry.at("id").get<Pixel::PixelEntityID>()] = Pixel::Solid{};
                }
            }

            if (pixelAttributes.contains("liquidAttributes")) {
                for (const auto& entry : pixelAttributes["liquidAttributes"]) {
                    Pixel::Liquid liquid{};
                    liquid.viscosity = entry.value("viscosity", 0.5f);
                    liquid.updateThisFrame = entry.value("updateThisFrame", false);
                    _pixelAttributes.liquidAttributes[entry.at("id").get<Pixel::PixelEntityID>()] = liquid;
                }
            }

            if (pixelAttributes.contains("gaseousAttributes")) {
                for (const auto& entry : pixelAttributes["gaseousAttributes"]) {
                    Pixel::Gaseous gaseous{};
                    gaseous.density = entry.value("density", 0.5f);
                    _pixelAttributes.gaseousAttributes[entry.at("id").get<Pixel::PixelEntityID>()] = gaseous;
                }
            }
        }

        if (scene.contains("chunkGrid") && scene["chunkGrid"].is_array()) {
            for (const auto& chunkEntry : scene["chunkGrid"]) {
                const int cx = chunkEntry.value("cx", 0);
                const int cy = chunkEntry.value("cy", 0);
                auto& chunk = _chunkGrid.getOrCreateChunk(cx, cy);

                if (!chunkEntry.contains("cells") || !chunkEntry["cells"].is_array())
                    continue;

                for (const auto& cell : chunkEntry["cells"]) {
                    const int x = cell.value("x", 0);
                    const int y = cell.value("y", 0);
                    const auto id = cell.value("id", Pixel::EMPTY);
                    chunk.set(x, y, id);
                }
            }
        }

        return true;
    }

} // namespace engine