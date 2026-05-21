#include <GL/glew.h>
#include <algorithm>
#include <cmath>
#include <engine/ecs/components/physicsComponent.hpp>
#include <engine/ecs/components/scriptComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/ecs/systems/movementSystem.hpp>
#include <engine/ecs/systems/physicsSystem.hpp>
#include <engine/ecs/systems/scriptSystem.hpp>
#include <engine/ecs/systems/spriteRenderSystem.hpp>
#include <engine/renderUtils.hpp>
#include <engine/scene/sceneSerializer.hpp>
#include <game/Game.hpp>
#include <iostream>
#include <tracy/Tracy.hpp>
#include <typeindex>

#ifndef TRACY_ENABLE
// output a warning if profiling is disabled
#pragma message(                                                                                   \
    "Tracy profiling is disabled. To enable, set PIKSELITE_ENABLE_PROFILING=ON in CMake and rebuild.")
#error "Not set"
#endif

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
        const size_t pairCount = std::min(go.pixelLocalCoords.size(), go.pixels.size());
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
    Game::Game(int width, int height, const std::string& title) : _pixelSimulation(_chunkGrid)
    {
        initSDL(width, height, title);
        _renderer = new graphics::Renderer(_window, _glContext);
        _boxWorld.init({0.0f, -500.0f});
        initECS();
        _pixelSimulation.setPhysicsWorld(_boxWorld.getWorldId(), PIXEL_SIZE);
    }

    Game::~Game()
    {
        delete _renderer;
        shutdownSDL();
    }

    void Game::initSDL(int width, int height, const std::string& title)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            std::cerr << "Game: SDL_Init failed: " << SDL_GetError() << std::endl;
            return;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        _window =
            SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
                             height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!_window)
        {
            std::cerr << "Game: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return;
        }

        _glContext = SDL_GL_CreateContext(_window);
        if (!_glContext)
        {
            std::cerr << "Game: SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(_window);
            SDL_Quit();
            return;
        }

        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK)
        {
            std::cerr << "Game: GLEW init failed: " << glewGetErrorString(glewError) << std::endl;
            SDL_GL_DeleteContext(_glContext);
            SDL_DestroyWindow(_window);
            SDL_Quit();
        }
    }

    void Game::shutdownSDL()
    {
        if (_glContext)
            SDL_GL_DeleteContext(_glContext);
        if (_window)
            SDL_DestroyWindow(_window);
        SDL_Quit();
    }

    void Game::initECS()
    {
        _componentManager.registerComponent<ecs::components::Transform>();
        _componentManager.registerComponent<ecs::components::Velocity>();
        _componentManager.registerComponent<ecs::components::Sprite>();
        _componentManager.registerComponent<ecs::components::PhysicsBody>();
        _componentManager.registerComponent<ecs::components::Script>();

        auto& movementSys = _systemManager.addSystem<ecs::systems::MovementSystem>();
        ecs::Signature movementSig;
        movementSig.set(_componentManager.getComponentType<ecs::components::Transform>());
        movementSig.set(_componentManager.getComponentType<ecs::components::Velocity>());
        _systemManager.setSignature<ecs::systems::MovementSystem>(movementSig);

        auto& spriteRenderSys =
            _systemManager.addSystem<ecs::systems::SpriteRenderSystem>(_renderer, &_camera);
        ecs::Signature spriteSig;
        spriteSig.set(_componentManager.getComponentType<ecs::components::Transform>());
        spriteSig.set(_componentManager.getComponentType<ecs::components::Sprite>());
        _systemManager.setSignature<ecs::systems::SpriteRenderSystem>(spriteSig);

        auto& scriptSys = _systemManager.addSystem<ecs::systems::ScriptSystem>(
            &_entityManager, &_systemManager, &_eventBus, &_pixelSimulation, &_camera);
        ecs::Signature scriptSig;
        scriptSig.set(_componentManager.getComponentType<ecs::components::Script>());
        _systemManager.setSignature<ecs::systems::ScriptSystem>(scriptSig);

        auto& physicsSys =
            _systemManager.addSystem<ecs::systems::PhysicsSystem>(&_boxWorld, &_eventBus);
        ecs::Signature physicsSig;
        physicsSig.set(_componentManager.getComponentType<ecs::components::Transform>());
        physicsSig.set(_componentManager.getComponentType<ecs::components::PhysicsBody>());
        _systemManager.setSignature<ecs::systems::PhysicsSystem>(physicsSig);

        _componentManager.setEntityMutationCallback([this](ecs::EntityID entityId)
                                                    { refreshEntitySignature(entityId); });

        _componentManager.setComponentRemovalCallback(
            [this](ecs::EntityID entityId, const std::type_index& componentType)
            {
                if (componentType == typeid(ecs::components::PhysicsBody))
                {
                    if (auto* physicsSys = _systemManager.getSystem<ecs::systems::PhysicsSystem>())
                    {
                        physicsSys->entityDestroyed(entityId);
                    }
                }
            });

        scriptSys.init();

        // Scripts request scene reloads via the event bus. Queue the target path;
        // run() drains it between frames so loadScene runs outside the ECS update.
        _eventBus.subscribe<events::SceneLoadRequestedEvent>(
            [this](const events::SceneLoadRequestedEvent& ev) { _pendingSceneLoadPath = ev.path; });
    }

    bool Game::loadScene(const std::string& filename)
    {
        engine::scene::SceneData data;
        if (!engine::scene::loadSceneFromFile(filename, data))
        {
            std::cerr << "Game: Failed to load scene: " << filename << std::endl;
            return false;
        }

        _currentScenePath = filename;
        _gameObjects = std::move(data.gameObjects);
        _gameObjectCounter = data.nextGameObjectId;

        _chunkGrid.chunks.clear();

        for (const auto& go : _gameObjects)
        {
            if (go.pixels.empty() || go.pixelLocalCoords.empty())
                continue;

            const auto* transform = go.getComponent<ecs::components::Transform>();
            if (!transform)
                continue;

            const int anchorGX = static_cast<int>(std::floor(transform->x / PIXEL_SIZE));
            const int anchorGY = static_cast<int>(std::floor(transform->y / PIXEL_SIZE));
            const size_t count = std::min(go.pixels.size(), go.pixelLocalCoords.size());

            for (size_t i = 0; i < count; ++i)
            {
                const auto& local = go.pixelLocalCoords[i];
                const auto& pixel = go.pixels[i];
                if (pixel.type == Element::EMPTY)
                    continue;

                const int gridX = anchorGX + local.x;
                const int gridY = anchorGY + local.y;
                Element::Pixel scenePixel;
                scenePixel.type = pixel.type;
                scenePixel.colorIndex = _renderer->generatePixelColorIndex(gridX, gridY);
                scenePixel.isBurning = pixel.isBurning;
                if (pixel.type == Element::FIRE)
                {
                    scenePixel.burnTimer = pixel.burnTimer != 0
                                               ? pixel.burnTimer
                                               : g_elements[Element::FIRE].fireParams.burnDuration;
                }
                else
                {
                    scenePixel.burnTimer = pixel.burnTimer;
                }
                _chunkGrid.setPixel(gridX, gridY, scenePixel);
            }
        }

        _camera.setPosition(data.cameraX, data.cameraY);
        _camera.setZoom(data.cameraZoom);

        _pixelSimulation.setGrid(_chunkGrid);
        loadGameObjectsIntoECS();
        return true;
    }

    void Game::loadGameObjectsIntoECS()
    {
        _systemManager.shutdownAll();

        const auto& allEntities = _entityManager.getEntities();
        std::vector<ecs::EntityID> entitiesToDestroy;
        for (const auto& entityPtr : allEntities)
        {
            if (entityPtr)
                entitiesToDestroy.push_back(entityPtr->id);
        }

        for (ecs::EntityID entityId : entitiesToDestroy)
        {
            _componentManager.entityDestroyed(entityId);
            _systemManager.entityDestroyed(entityId);
            _entityManager.destroyEntity(ecs::Entity(entityId));
        }

        _gameObjectToEntity.clear();
        _gameObjectOccupiedCells.clear();

        for (const auto& go : _gameObjects)
        {
            ecs::Entity entity = _entityManager.createEntity();
            ecs::EntityID eid = entity.id;

            if (auto* scriptSys = _systemManager.getSystem<ecs::systems::ScriptSystem>())
            {
                const std::string runtimeName =
                    go.name.empty() ? ("GameObject_" + std::to_string(go.id)) : go.name;
                scriptSys->setEntityName(eid, runtimeName);
            }

            for (const auto& [compType, compData] : go.components)
            {
                if (compType == std::type_index(typeid(ecs::components::Transform)))
                {
                    const auto& t = std::any_cast<ecs::components::Transform>(compData);
                    _componentManager.addComponent(eid, t);
                }
                else if (compType == std::type_index(typeid(ecs::components::Velocity)))
                {
                    const auto& v = std::any_cast<ecs::components::Velocity>(compData);
                    _componentManager.addComponent(eid, v);
                }
                else if (compType == std::type_index(typeid(ecs::components::Sprite)))
                {
                    const auto& s = std::any_cast<ecs::components::Sprite>(compData);
                    _componentManager.addComponent(eid, s);
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
                    _componentManager.addComponent(eid, p);
                }
                else if (compType == std::type_index(typeid(ecs::components::Script)))
                {
                    const auto& s = std::any_cast<ecs::components::Script>(compData);
                    _componentManager.addComponent(eid, s);
                }
            }

            _gameObjectToEntity[go.id] = eid;
        }
    }

    void Game::syncGameObjectPixelsFromPhysics()
    {
        ChunkGrid& grid = _pixelSimulation.getGrid();

        std::unordered_map<Pixel::GameObjectID, std::vector<Element::Vec2i>> nextOccupiedCells;
        nextOccupiedCells.reserve(_gameObjects.size());

        for (auto& go : _gameObjects)
        {
            if (!go.isActive)
                continue;
            if (go.pixelLocalCoords.empty() || go.pixels.empty())
                continue;

            auto entityIt = _gameObjectToEntity.find(go.id);
            if (entityIt == _gameObjectToEntity.end())
                continue;

            const ecs::EntityID entityId = entityIt->second;
            if (!_componentManager.hasComponent<ecs::components::Transform>(entityId))
                continue;
            if (!_componentManager.hasComponent<ecs::components::PhysicsBody>(entityId))
                continue;

            auto& physics = _componentManager.getComponent<ecs::components::PhysicsBody>(entityId);
            if (!physics.enabled)
                continue;

            const auto& transform =
                _componentManager.getComponent<ecs::components::Transform>(entityId);
            const float cosine = std::cos(transform.rotation);
            const float sine = std::sin(transform.rotation);

            const size_t pairCount = std::min(go.pixelLocalCoords.size(), go.pixels.size());
            std::vector<Element::Vec2i> occupiedCells;
            occupiedCells.reserve(pairCount);

            std::vector<Element::Pixel> nextPixels;
            std::vector<Element::Vec2i> nextLocalCoords;
            nextPixels.reserve(pairCount);
            nextLocalCoords.reserve(pairCount);

            const auto prevIt = _gameObjectOccupiedCells.find(go.id);
            const std::vector<Element::Vec2i>* prevCells =
                (prevIt != _gameObjectOccupiedCells.end()) ? &prevIt->second : nullptr;
            const size_t prevCount = prevCells ? prevCells->size() : 0;

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

                Element::Pixel stampedPixel{srcPixel.type, false, srcPixel.colorIndex,
                                            srcPixel.burnTimer, srcPixel.isBurning};

                const bool hasPrevCell = (prevCells && i < prevCount);
                Element::Vec2i prevCell{};
                Element::Pixel prevPixel{Element::EMPTY};
                if (hasPrevCell)
                {
                    prevCell = (*prevCells)[i];
                    prevPixel = grid.getPixel(prevCell.x, prevCell.y);
                }

                Element::Pixel currentPixel = grid.getPixel(gridX, gridY);
                const bool pixelAlive = (currentPixel.type != Element::EMPTY) ||
                                        (hasPrevCell && prevPixel.type != Element::EMPTY);
                if (!pixelAlive)
                {
                    if (hasPrevCell && (prevCell.x != gridX || prevCell.y != gridY))
                    {
                        grid.setPixel(prevCell.x, prevCell.y, {Element::EMPTY, false});
                    }
                    continue;
                }

                if (hasPrevCell && prevPixel.type != Element::EMPTY)
                {
                    stampedPixel = prevPixel;
                }

                if (hasPrevCell && (prevCell.x != gridX || prevCell.y != gridY))
                {
                    grid.setPixel(prevCell.x, prevCell.y, {Element::EMPTY, false});
                }

                grid.setPixel(gridX, gridY, stampedPixel);
                occupiedCells.push_back({gridX, gridY});
                nextPixels.push_back(srcPixel);
                nextLocalCoords.push_back(go.pixelLocalCoords[i]);
            }

            if (prevCells && prevCount > pairCount)
            {
                for (size_t i = pairCount; i < prevCount; ++i)
                {
                    const auto& prevCell = (*prevCells)[i];
                    grid.setPixel(prevCell.x, prevCell.y, {Element::EMPTY, false});
                }
            }

            if (!occupiedCells.empty())
            {
                nextOccupiedCells[go.id] = std::move(occupiedCells);
            }

            go.pixels = std::move(nextPixels);
            go.pixelLocalCoords = std::move(nextLocalCoords);

            if (go.pixelCount != go.pixels.size())
            {
                std::vector<ecs::components::PhysicsTriangle> generatedTriangles;
                if (buildPhysicsTrianglesFromGameObjectPixels(go, _pixelSimulation,
                                                              generatedTriangles))
                {
                    physics.triangles = std::move(generatedTriangles);
                }
                else
                {
                    physics.triangles.clear();
                }

                if (auto* physicsSys = _systemManager.getSystem<ecs::systems::PhysicsSystem>())
                {
                    physicsSys->entityDestroyed(entityId);
                }
                physics.bodyId = b2_nullBodyId;
            }

            go.pixelCount = go.pixels.size();
        }

        _gameObjectOccupiedCells = std::move(nextOccupiedCells);
    }

    void Game::refreshEntitySignature(ecs::EntityID entityId)
    {
        if (!_entityManager.hasEntity(entityId))
            return;

        ecs::Signature sig = _componentManager.getEntitySignature(entityId);
        _entityManager.setSignature(entityId, sig);
        _systemManager.entitySignatureChanged(entityId, sig);
    }

    std::vector<graphics::Pixel> Game::buildRenderPixels(const ChunkGrid& grid) const
    {
        return engine::buildRenderPixels(grid);
    }

    void Game::handleEvents()
    {
        ZoneScopedN("Game::Events");
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent))
        {
            if (sdlEvent.type == SDL_QUIT)
            {
                _running = false;
            }
            else if (sdlEvent.type == SDL_WINDOWEVENT &&
                     sdlEvent.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                _running = false;
            }
            else if (sdlEvent.type == SDL_KEYDOWN)
            {
                if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
                {
                    _running = false;
                }
            }
        }
    }

    void Game::update(float deltaTime)
    {
        ZoneScopedN("Game::Update");
        _accumulator += deltaTime;
        if (_accumulator > 0.25f)
            _accumulator = 0.25f;

        while (_accumulator >= _fixedDt)
        {
            ZoneScopedN("Game::PixelSim");
            _pixelSimulation.update();
            _accumulator -= _fixedDt;
        }

        _systemManager.update(deltaTime, _componentManager);
        syncGameObjectPixelsFromPhysics();
    }

    void Game::drawSpritesBelowLayer(int layer)
    {
        struct SpriteDrawItem
        {
            int layer;
            ecs::EntityID entityId;
        };

        std::vector<SpriteDrawItem> drawList;
        const auto& entities = _entityManager.getEntities();
        drawList.reserve(entities.size());

        for (const auto& entityPtr : entities)
        {
            if (!entityPtr)
                continue;
            const ecs::EntityID entityId = entityPtr->id;

            if (!_componentManager.hasComponent<ecs::components::Transform>(entityId))
                continue;
            if (!_componentManager.hasComponent<ecs::components::Sprite>(entityId))
                continue;

            auto& sprite = _componentManager.getComponent<ecs::components::Sprite>(entityId);
            auto& transform = _componentManager.getComponent<ecs::components::Transform>(entityId);

            if (!sprite.enabled || !transform.enabled)
                continue;
            if (sprite.layer >= layer)
                continue;

            drawList.push_back({sprite.layer, entityId});
        }

        std::sort(drawList.begin(), drawList.end(),
                  [](const SpriteDrawItem& a, const SpriteDrawItem& b)
                  {
                      if (a.layer != b.layer)
                          return a.layer < b.layer;
                      return a.entityId < b.entityId;
                  });

        for (const auto& item : drawList)
        {
            auto& sprite = _componentManager.getComponent<ecs::components::Sprite>(item.entityId);
            auto& transform =
                _componentManager.getComponent<ecs::components::Transform>(item.entityId);

            if (!sprite.loaded && !sprite.texturePath.empty())
            {
                sprite.textureID = _renderer->loadTexture(sprite.texturePath);
                sprite.loaded = true;
            }

            if (sprite.textureID == 0)
                continue;

            graphics::Sprite2D s2d;
            s2d.position = {transform.x, transform.y};
            s2d.size = {sprite.width * transform.scaleX, sprite.height * transform.scaleY};
            s2d.textureID = sprite.textureID;

            _renderer->drawSprite(s2d, _camera);
        }
    }

    void Game::drawSpritesAboveLayer(int layer)
    {
        struct SpriteDrawItem
        {
            int layer;
            ecs::EntityID entityId;
        };

        std::vector<SpriteDrawItem> drawList;
        const auto& entities = _entityManager.getEntities();
        drawList.reserve(entities.size());

        for (const auto& entityPtr : entities)
        {
            if (!entityPtr)
                continue;
            const ecs::EntityID entityId = entityPtr->id;

            if (!_componentManager.hasComponent<ecs::components::Transform>(entityId))
                continue;
            if (!_componentManager.hasComponent<ecs::components::Sprite>(entityId))
                continue;

            auto& sprite = _componentManager.getComponent<ecs::components::Sprite>(entityId);
            auto& transform = _componentManager.getComponent<ecs::components::Transform>(entityId);

            if (!sprite.enabled || !transform.enabled)
                continue;
            if (sprite.layer <= layer)
                continue;

            drawList.push_back({sprite.layer, entityId});
        }

        std::sort(drawList.begin(), drawList.end(),
                  [](const SpriteDrawItem& a, const SpriteDrawItem& b)
                  {
                      if (a.layer != b.layer)
                          return a.layer < b.layer;
                      return a.entityId < b.entityId;
                  });

        for (const auto& item : drawList)
        {
            auto& sprite = _componentManager.getComponent<ecs::components::Sprite>(item.entityId);
            auto& transform =
                _componentManager.getComponent<ecs::components::Transform>(item.entityId);

            if (!sprite.loaded && !sprite.texturePath.empty())
            {
                sprite.textureID = _renderer->loadTexture(sprite.texturePath);
                sprite.loaded = true;
            }

            if (sprite.textureID == 0)
                continue;

            graphics::Sprite2D s2d;
            s2d.position = {transform.x, transform.y};
            s2d.size = {sprite.width * transform.scaleX, sprite.height * transform.scaleY};
            s2d.textureID = sprite.textureID;

            _renderer->drawSprite(s2d, _camera);
        }
    }

    void Game::render()
    {
        ZoneScopedN("Game::Render");
        std::vector<graphics::Pixel> framePixels = buildRenderPixels(_pixelSimulation.getGrid());

        _renderer->clear();
        drawSpritesBelowLayer(0);
        _renderer->drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);
        _renderer->drawParticlesWCamera(_pixelSimulation.getParticles(), _camera, PIXEL_SIZE);

        if (auto* physicsSystem = _systemManager.getSystem<ecs::systems::PhysicsSystem>())
        {
            const std::vector<b2BodyId> ecsDebugBodies = physicsSystem->getDebugBodies();
            _renderer->drawBox2DDebug(_boxWorld.getWorldId(), ecsDebugBodies, _camera, 1.0f,
                                      glm::vec3(1.0f, 0.8f, 0.2f));
        }

        drawSpritesAboveLayer(0);

        _renderer->present(_window);
    }

    void Game::run()
    {
        ZoneScopedN("Game::Run");
        while (_running)
        {
            _timer.tick();
            handleEvents();
            update(_timer.getDeltaTime());

            // Drain any reload_scene() / load_scene(path) dispatched on the event bus
            // during this frame.
            if (_pendingSceneLoadPath)
            {
                const std::string target =
                    _pendingSceneLoadPath->empty() ? _currentScenePath : *_pendingSceneLoadPath;
                _pendingSceneLoadPath.reset();
                if (!target.empty() && loadScene(target))
                {
                    _accumulator = 0.0f; // discard physics catch-up from the old scene
                }
            }

            render();
            FrameMark;
        }
    }
} // namespace engine
