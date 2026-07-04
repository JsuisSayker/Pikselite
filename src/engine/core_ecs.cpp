#include <algorithm>
#include <cmath>
#include <engine/core.hpp>
#include <engine/ecs/components/physicsComponent.hpp>
#include <engine/ecs/components/scriptComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/ecs/systems/movementSystem.hpp>
#include <engine/ecs/systems/physicsSystem.hpp>
#include <engine/ecs/systems/scriptSystem.hpp>
#include <engine/ecs/systems/spriteRenderSystem.hpp>
#include <typeindex>

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
    bool Core::copyProjectEditorDataToCore()
    {
        if (!isProjectEditorActive)
            return false;

        _renderPixels = projectEditor->getPixels();
        _gameObjects = projectEditor->getGameObjects();
        _chunkGrid = projectEditor->getChunkGrid();
        _pixelSimulation.setGrid(_chunkGrid);
        gameObjectCounter = projectEditor->getGameObjectCounter();

        {
            graphics::Camera2D editorCam = projectEditor->getCamera();
            _camera.setPosition(editorCam.getPosition().x, editorCam.getPosition().y);
            _camera.setZoom(editorCam.getZoom());
        }

        loadGameObjectsIntoECS();

        return true;
    }

    void Core::init()
    {
        _boxWorld.init({0.0f, -500.0f});

        getJsonVariables();
        loadProjects(_projects);

        // Register ECS components
        componentManager.registerComponent<ecs::components::Transform>();
        componentManager.registerComponent<ecs::components::Velocity>();
        componentManager.registerComponent<ecs::components::Sprite>();
        componentManager.registerComponent<ecs::components::PhysicsBody>();
        componentManager.registerComponent<ecs::components::Script>();

        // Register ECS systems
        auto& movementSys = systemManager.addSystem<ecs::systems::MovementSystem>();
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
            &entityManager, &systemManager, &eventBus, &_pixelSimulation, &_camera);
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

        // Scripts request scene reloads via the event bus. Queue the target path
        // here; runGamePreview drains it between frames so loadScene runs outside
        // the ECS update.
        eventBus.subscribe<events::SceneLoadRequestedEvent>(
            [this](const events::SceneLoadRequestedEvent& ev) { _pendingSceneLoadPath = ev.path; });

        spriteEditor = new editors::SpriteEditor(&sdlInterface, &renderer, &imguiInterface);
        projectEditor = new editors::ProjectEditor(&sdlInterface, &renderer, &imguiInterface,
                                                   &componentManager);

        _pixelSimulation.setPhysicsWorld(_boxWorld.getWorldId(), PIXEL_SIZE);

    }

    void Core::loadGameObjectsIntoECS()
    {
        // Shutdown all systems to clear their state before reloading
        systemManager.shutdownAll();

        // Destroy ALL entities (including dynamically created ones from Lua)
        const auto& allEntities = entityManager.getEntities();
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
        // Stone-region body wrappers were just destroyed in the loop above. Drop
        // their stale IDs too: the EntityManager recycles freed IDs, so leaving
        // them here makes the next syncRegionBodiesToECS() destroy whatever real
        // GameObject entity got the recycled ID — which then leaves _gameObjectToEntity
        // pointing at a region body sitting at the origin, stamping that GameObject's
        // pixels into the middle of the scene ("stone mountain" bug on preview re-entry).
        _regionBodyEntities.clear();

        for (const auto& go : _gameObjects)
        {
            ecs::Entity entity = entityManager.createEntity();
            ecs::EntityID eid = entity.id;

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
            if (!componentManager.hasComponent<ecs::components::Transform>(entityId))
                continue;
            if (!componentManager.hasComponent<ecs::components::PhysicsBody>(entityId))
                continue;

            auto& physics = componentManager.getComponent<ecs::components::PhysicsBody>(entityId);
            if (!physics.enabled)
                continue;

            const auto& transform =
                componentManager.getComponent<ecs::components::Transform>(entityId);
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

                if (auto* physicsSys = systemManager.getSystem<ecs::systems::PhysicsSystem>())
                {
                    physicsSys->entityDestroyed(entityId);
                }
                physics.bodyId = b2_nullBodyId;
            }

            go.pixelCount = go.pixels.size();
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
} // namespace engine
