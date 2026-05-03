#include <cmath>
#include <engine/core.hpp>
#include <engine/ecs/components/physicsComponent.hpp>
#include <engine/ecs/components/scriptComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/ecs/systems/movementSystem.hpp>
#include <tracy/Tracy.hpp>

#ifndef TRACY_ENABLE
// output a warning if profiling is disabled
#pragma message(                                                                                   \
    "Tracy profiling is disabled. To enable, set PIKSELITE_ENABLE_PROFILING=ON in CMake and rebuild.")
#error "Not set"
#endif
#include <box2d/box2d.h>
#include <engine/ecs/systems/physicsSystem.hpp>
#include <engine/ecs/systems/scriptSystem.hpp>
#include <engine/ecs/systems/spriteRenderSystem.hpp>
#include <filesystem>
#include <fstream>
#include <imgui.h>

namespace
{

    bool buildPhysicsTrianglesFromGameObjectPixels(
        const Pixel::GameObject& go, Simulation& simulation,
        std::vector<ecs::components::PhysicsTriangle>& outTriangles)
    {
        outTriangles.clear();
        if (go.pixelLocalCoords.empty())
            return false;

        Element::Region region;
        const size_t    pairCount = std::min(go.pixelLocalCoords.size(), go.pixels.size());
        if (pairCount == 0)
            return false;

        region.pixels.reserve(pairCount);
        for (size_t i = 0; i < pairCount; ++i)
        {
            const auto& srcPixel = go.pixels[i];
            if (srcPixel.type == Element::EMPTY)
                continue;

            const auto& def = g_elements[srcPixel.type];
            if (def.state != SOLID_STATIC)
                continue;

            region.pixels.push_back(go.pixelLocalCoords[i]);
        }

        if (region.pixels.empty())
            return false;

        simulation.buildRegionContoursMarchingSquare(region);
        simulation.simplifyRegionContours(region, 0.6f);
        simulation.triangulateRegion(region);

        if (region.triangles.empty())
            return false;

        outTriangles.reserve(region.triangles.size());
        for (const auto& tri : region.triangles)
        {
            ecs::components::PhysicsTriangle physicsTri;
            physicsTri.a = glm::vec2(tri.a.x * PIXEL_SIZE, tri.a.y * PIXEL_SIZE);
            physicsTri.b = glm::vec2(tri.b.x * PIXEL_SIZE, tri.b.y * PIXEL_SIZE);
            physicsTri.c = glm::vec2(tri.c.x * PIXEL_SIZE, tri.c.y * PIXEL_SIZE);
            outTriangles.push_back(physicsTri);
        }

        return true;
    }
} // namespace

namespace engine
{
    void Core::init()
    {
        _boxWorld.init({0.0f, -500.0f});

        // Register ECS components
        componentManager.registerComponent<ecs::components::Transform>();
        componentManager.registerComponent<ecs::components::Velocity>();
        componentManager.registerComponent<ecs::components::Sprite>();
        componentManager.registerComponent<ecs::components::PhysicsBody>();
        componentManager.registerComponent<ecs::components::Script>();

        // Register ECS systems
        auto&          movementSys = systemManager.addSystem<ecs::systems::MovementSystem>();
        ecs::Signature movementSig;
        movementSig.set(componentManager.getComponentType<ecs::components::Transform>());
        movementSig.set(componentManager.getComponentType<ecs::components::Velocity>());
        systemManager.setSignature<ecs::systems::MovementSystem>(movementSig);

        // Sprite render system (needs Transform + Sprite)
        auto& spriteRenderSys =
            systemManager.addSystem<ecs::systems::SpriteRenderSystem>(&renderer, &_camera);
        ecs::Signature spriteSig;
        spriteSig.set(componentManager.getComponentType<ecs::components::Transform>());
        spriteSig.set(componentManager.getComponentType<ecs::components::Sprite>());
        systemManager.setSignature<ecs::systems::SpriteRenderSystem>(spriteSig);

        // Script system (needs Transform + Velocity) — runs Lua scripts
        auto& scriptSys = systemManager.addSystem<ecs::systems::ScriptSystem>(
            &entityManager, &systemManager, &eventBus, &_chunkGrid, &_camera);
        ecs::Signature scriptSig;
        scriptSig.set(componentManager.getComponentType<ecs::components::Script>());
        systemManager.setSignature<ecs::systems::ScriptSystem>(scriptSig);

        auto& physicsSys =
            systemManager.addSystem<ecs::systems::PhysicsSystem>(&_boxWorld, &eventBus);
        ecs::Signature physicsSig;
        physicsSig.set(componentManager.getComponentType<ecs::components::Transform>());
        physicsSig.set(componentManager.getComponentType<ecs::components::PhysicsBody>());
        systemManager.setSignature<ecs::systems::PhysicsSystem>(physicsSig);

        // Centralized signature sync: any component add/remove updates system membership.
        componentManager.setEntityMutationCallback([this](ecs::EntityID entityId)
                                                   { refreshEntitySignature(entityId); });

        componentManager.setComponentRemovalCallback(
            [this](ecs::EntityID entityId, const std::type_index& componentType)
            {
                if (componentType == typeid(ecs::components::PhysicsBody))
                {
                    if (auto* physicsSys = systemManager.getSystem<ecs::systems::PhysicsSystem>())
                    {
                        physicsSys->entityDestroyed(entityId);
                    }
                }
            });

        scriptSys.init();

        spriteEditor  = new editors::SpriteEditor(&sdlInterface, &renderer, &imguiInterface);
        projectEditor = new editors::ProjectEditor(&sdlInterface, &renderer, &imguiInterface,
                                                   &componentManager);

        _pixelSimulation.setPhysicsWorld(_boxWorld.getWorldId(), PIXEL_SIZE);
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

                if (projectEditor->consumeSaveSceneRequest())
                {
                    copyProjectEditorDataToCore();
                    saveScene(_sceneFilename);
                }

                if (projectEditor->consumeLoadSceneRequest())
                {
                    if (loadScene(_sceneFilename))
                    {
                        projectEditor->setSceneData(_renderPixels, _gameObjects, _chunkGrid,
                                                    gameObjectCounter);
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
        SDL_Window* gameWindow =
            SDL_CreateWindow("Game Preview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        SDL_GL_MakeCurrent(gameWindow, sdlInterface.getGLContext());

        copyProjectEditorDataToCore();
        // _pixelSimulation.markRegionsDirty();

        while (isGamePreviewActive && running)
        {
            timer.tick();
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

            update(timer.getDeltaTime());

            renderer.clear();

            std::vector<graphics::Pixel> framePixels =
                buildRenderPixels(_pixelSimulation.getGrid());

            _renderPixels = framePixels;
            renderer.drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);

            // debug draw Box2D bodies
            if (auto* physicsSystem = systemManager.getSystem<ecs::systems::PhysicsSystem>())
            {
                const std::vector<b2BodyId> ecsDebugBodies = physicsSystem->getDebugBodies();
                renderer.drawBox2DDebug(_boxWorld.getWorldId(), ecsDebugBodies, _camera, 1.0f,
                                        glm::vec3(1.0f, 0.8f, 0.2f));
            }

            if (auto* spriteSystem = systemManager.getSystem<ecs::systems::SpriteRenderSystem>())
            {
                spriteSystem->update(0.0, componentManager);
            }

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
                {
                    // Ensure preview reads the latest pixels/chunks from the editor state.
                    copyProjectEditorDataToCore();
                    isGamePreviewActive             = true;
                    graphics::Camera2D editorCamera = projectEditor->getCamera();
                    setCameraPosition(editorCamera.getPosition().x, editorCamera.getPosition().y);
                    setCameraZoom(editorCamera.getZoom());
                }
                break;
            default:
                break;
        }
        return event;
    }

    void Core::update(float deltaTime)
    {
        ZoneScoped;

        accumulator += deltaTime;

        if (accumulator > 0.25f)
            accumulator = 0.25f;

        while (accumulator >= fixedDt)
        {
            {
                ZoneScopedN("PixelSimulation");
                _pixelSimulation.update();
            }

            accumulator -= fixedDt;
        }

        // Single ECS pass: any newly registered system is updated automatically.
        {
            ZoneScopedN("ECS Systems");
            systemManager.update(deltaTime, componentManager);
        }

        syncGameObjectPixelsFromPhysics();
    }

    void Core::render()
    {
        renderer.clear();
        renderer.drawPixelsOverlay(_renderPixels, PIXEL_SIZE);
        renderer.present(sdlInterface.getWindow());
    }

    void Core::shutdown()
    {
        if (projectEditor)
        {
            copyProjectEditorDataToCore();
            saveScene(_sceneFilename);
        }
        _boxWorld.shutdown();
        SDL_Quit();
    }

    bool Core::copyProjectEditorDataToCore()
    {
        if (!isProjectEditorActive)
            return false;

        _renderPixels = projectEditor->getPixels();
        _gameObjects  = projectEditor->getGameObjects();
        _chunkGrid    = projectEditor->getChunkGrid();
        _pixelSimulation.setGrid(_chunkGrid);
        gameObjectCounter = projectEditor->getGameObjectCounter();

        loadGameObjectsIntoECS();

        return true;
    }

    void Core::loadGameObjectsIntoECS()
    {
        // Shutdown all systems to clear their state before reloading
        systemManager.shutdownAll();

        // Destroy ALL entities (including dynamically created ones from Lua)
        const auto&                allEntities = entityManager.getEntities();
        std::vector<ecs::EntityID> entitiesToDestroy;
        for (const auto& entityPtr : allEntities)
        {
            if (entityPtr)
            {
                entitiesToDestroy.push_back(entityPtr->id);
            }
        }

        for (ecs::EntityID entityId : entitiesToDestroy)
        {
            componentManager.entityDestroyed(entityId);
            systemManager.entityDestroyed(entityId);
            entityManager.destroyEntity(ecs::Entity(entityId));
        }

        _gameObjectToEntity.clear();
        _gameObjectOccupiedCells.clear();

        for (const auto& go : _gameObjects)
        {
            ecs::Entity   entity = entityManager.createEntity();
            ecs::EntityID eid    = entity.id;

            if (auto* scriptSys = systemManager.getSystem<ecs::systems::ScriptSystem>())
            {
                const std::string runtimeName =
                    go.name.empty() ? ("GameObject_" + std::to_string(go.id)) : go.name;
                scriptSys->setEntityName(eid, runtimeName);
            }

            // loop on components in game object and add to ECS entity
            for (const auto& [compType, compData] : go.components)
            {
                if (compType == std::type_index(typeid(ecs::components::Transform)))
                {
                    const auto& t = std::any_cast<ecs::components::Transform>(compData);
                    componentManager.addComponent(eid, t);
                }
                else if (compType == std::type_index(typeid(ecs::components::Velocity)))
                {
                    const auto& v = std::any_cast<ecs::components::Velocity>(compData);
                    componentManager.addComponent(eid, v);
                }
                else if (compType == std::type_index(typeid(ecs::components::Sprite)))
                {
                    const auto& s = std::any_cast<ecs::components::Sprite>(compData);
                    componentManager.addComponent(eid, s);
                }
                else if (compType == std::type_index(typeid(ecs::components::PhysicsBody)))
                {
                    auto p = std::any_cast<ecs::components::PhysicsBody>(compData);

                    if (p.triangles.empty())
                    {
                        std::vector<ecs::components::PhysicsTriangle> generatedTriangles;
                        if (buildPhysicsTrianglesFromGameObjectPixels(go, _pixelSimulation,
                                                                      generatedTriangles))
                        {
                            p.triangles = std::move(generatedTriangles);
                        }
                    }

                    componentManager.addComponent(eid, p);
                }
                else if (compType == std::type_index(typeid(ecs::components::Script)))
                {
                    const auto& s = std::any_cast<ecs::components::Script>(compData);
                    componentManager.addComponent(eid, s);
                }
            }

            _gameObjectToEntity[go.id] = eid;
        }
    }

    void Core::syncGameObjectPixelsFromPhysics()
    {
        ChunkGrid& grid = _pixelSimulation.getGrid();

        for (const auto& [goId, occupiedCells] : _gameObjectOccupiedCells)
        {
            for (const auto& cell : occupiedCells)
            {
                grid.setPixel(cell.x, cell.y, {Element::EMPTY, false});
            }
        }

        std::unordered_map<Pixel::GameObjectID, std::vector<Element::Vec2i>> nextOccupiedCells;
        nextOccupiedCells.reserve(_gameObjects.size());

        for (const auto& go : _gameObjects)
        {
            if (!go.isActive)
                continue;
            if (go.pixelLocalCoords.empty() || go.pixels.empty())
                continue;

            auto entityIt = _gameObjectToEntity.find(go.id);
            if (entityIt == _gameObjectToEntity.end())
                continue;

            const ecs::EntityID entityId = entityIt->second;
            if (!componentManager.hasComponent<ecs::components::Transform>(entityId))
                continue;
            if (!componentManager.hasComponent<ecs::components::PhysicsBody>(entityId))
                continue;

            const auto& physics =
                componentManager.getComponent<ecs::components::PhysicsBody>(entityId);
            if (!physics.enabled)
                continue;
            if (physics.bodyType != b2_dynamicBody)
                continue;

            const auto& transform =
                componentManager.getComponent<ecs::components::Transform>(entityId);
            const float cosine = std::cos(transform.rotation);
            const float sine   = std::sin(transform.rotation);

            const size_t pairCount = std::min(go.pixelLocalCoords.size(), go.pixels.size());
            std::vector<Element::Vec2i> occupiedCells;
            occupiedCells.reserve(pairCount);

            for (size_t i = 0; i < pairCount; ++i)
            {
                const auto& srcPixel = go.pixels[i];
                if (srcPixel.type == Element::EMPTY)
                    continue;

                const auto& def = g_elements[srcPixel.type];
                if (def.state != SOLID_STATIC)
                    continue;

                const float localX =
                    (static_cast<float>(go.pixelLocalCoords[i].x) + 0.5f) * PIXEL_SIZE;
                const float localY =
                    (static_cast<float>(go.pixelLocalCoords[i].y) + 0.5f) * PIXEL_SIZE;

                const float worldX = transform.x + (localX * cosine - localY * sine);
                const float worldY = transform.y + (localX * sine + localY * cosine);

                const int gridX = static_cast<int>(std::floor(worldX / PIXEL_SIZE));
                const int gridY = static_cast<int>(std::floor(worldY / PIXEL_SIZE));

                Element::Pixel existing = grid.getPixel(gridX, gridY);
                if (existing.type != Element::EMPTY && existing.type != srcPixel.type)
                {
                    _pixelSimulation.tryDisplacePixel(gridX, gridY, 3);
                }

                grid.setPixel(gridX, gridY, {srcPixel.type, false});
                occupiedCells.push_back({gridX, gridY});
            }

            if (!occupiedCells.empty())
            {
                nextOccupiedCells[go.id] = std::move(occupiedCells);
            }
        }

        _gameObjectOccupiedCells = std::move(nextOccupiedCells);
    }

    void Core::refreshEntitySignature(ecs::EntityID entityId)
    {
        if (!entityManager.hasEntity(entityId))
            return;

        ecs::Signature sig = componentManager.getEntitySignature(entityId);

        entityManager.setSignature(entityId, sig);
        systemManager.entitySignatureChanged(entityId, sig);
    }

    std::vector<graphics::Pixel> Core::buildRenderPixels(ChunkGrid grid) const
    {
        std::vector<graphics::Pixel> result;
        result.reserve(10000);

        for (const auto& [key, chunk] : grid.chunks)
        {
            // Correct signed decode from packed int64 key
            const int cx = static_cast<int32_t>(key >> 32);
            const int cy = static_cast<int32_t>(key & 0xFFFFFFFF);

            for (int y = 0; y < CHUNK_SIZE; ++y)
            {
                for (int x = 0; x < CHUNK_SIZE; ++x)
                {
                    const Element::Pixel& simPixel = chunk.pixels[y * CHUNK_SIZE + x];
                    if (simPixel.type == Element::EMPTY)
                        continue;

                    const auto& def = g_elements[simPixel.type];

                    graphics::Pixel renderPixel;

                    // grid -> world (apply chunk offset + pixel size)
                    const float gx = static_cast<float>(cx * CHUNK_SIZE + x);
                    const float gy = static_cast<float>(cy * CHUNK_SIZE + y);

                    renderPixel.position = glm::vec2(gx * PIXEL_SIZE, gy * PIXEL_SIZE);

                    if (simPixel.isBurning)
                    {
                        glm::vec3 pColor = glm::vec3(
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].r / 255.0f,
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].g / 255.0f,
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].b / 255.0f);
                        ElementDefinition& fireDef = g_elements[Element::FIRE];
                        glm::vec3          fColor  = glm::vec3(
                            fireDef.colorPalette[simPixel.colorIndex % PALETTE_SIZE].r / 255.0f,
                            fireDef.colorPalette[simPixel.colorIndex % PALETTE_SIZE].g / 255.0f,
                            fireDef.colorPalette[simPixel.colorIndex % PALETTE_SIZE].b / 255.0f);
                        float progress    = (def.fireParams.burnDuration > 0)
                                                ? 1.0f - (static_cast<float>(simPixel.burnTimer) /
                                                          def.fireParams.burnDuration)
                                                : 1.0f;
                        progress          = glm::clamp(progress, 0.0f, 1.0f);
                        renderPixel.color = glm::mix(pColor, fColor, progress);
                        renderPixel.color =
                            glm::clamp(renderPixel.color, glm::vec3(0.0f), glm::vec3(1.0f));
                    }
                    else
                    {
                        renderPixel.color = glm::vec3(
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].r / 255.0f,
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].g / 255.0f,
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].b / 255.0f);
                    }

                    result.push_back(renderPixel);
                }
            }
        }

        return result;
    }

    void Core::saveScene(const std::string& filename)
    {
        // TODO
    }

    bool Core::loadScene(const std::string& filename)
    {
        // TODO
        return false;
    }

} // namespace engine