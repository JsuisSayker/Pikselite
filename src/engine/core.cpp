#include <engine/core.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/ecs/components/gameObjectComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/systems/movementSystem.hpp>
#include <tracy/Tracy.hpp>

#include <cmath>
#include <random>

#ifndef TRACY_ENABLE
// output a warning if profiling is disabled
#pragma message("Tracy profiling is disabled. To enable, set PIKSELITE_ENABLE_PROFILING=ON in CMake and rebuild.")
#error "Not set"
#endif
#include <engine/ecs/systems/spriteRenderSystem.hpp>
#include <engine/ecs/systems/scriptSystem.hpp>
#include <fstream>
#include <filesystem>

namespace
{
    json pixelToJson(const graphics::Pixel &pixel)
    {
        return {
            {"x", pixel.position.x},
            {"y", pixel.position.y},
            {"r", pixel.color.r},
            {"g", pixel.color.g},
            {"b", pixel.color.b}};
    }

    graphics::Pixel pixelFromJson(const json &j)
    {
        return {
            {j.value("x", 0.0f), j.value("y", 0.0f)},
            {j.value("r", 1.0f), j.value("g", 1.0f), j.value("b", 1.0f)}};
    }

    json gameObjectToJson(const Pixel::GameObject &go)
    {
        // TODO
    }

    Pixel::GameObject gameObjectFromJson(const json &j)
    {
        // TODO
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

        // Centralized signature sync: any component add/remove updates system membership.
        componentManager.setEntityMutationCallback([this](ecs::EntityID entityId)
                                                   { refreshEntitySignature(entityId); });

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

                if (projectEditor->consumeSaveSceneRequest())
                {
                    copyProjectEditorDataToCore();
                    saveScene(_sceneFilename);
                }

                if (projectEditor->consumeLoadSceneRequest())
                {
                    if (loadScene(_sceneFilename))
                    {
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
        enum class DemoShapeType
        {
            Box,
            Circle,
        };

        struct DemoShape
        {
            b2BodyId bodyId = b2_nullBodyId;
            DemoShapeType type = DemoShapeType::Box;
            glm::vec2 size = {50.0f, 50.0f};
            float radius = 25.0f;
            glm::vec3 color = {1.0f, 0.0f, 0.0f};
        };

        const auto buildCirclePixels = [](glm::vec2 center, float radius, glm::vec3 color)
        {
            std::vector<graphics::Pixel> pixels;
            for (float y = -radius + PIXEL_SIZE * 0.5f; y < radius; y += PIXEL_SIZE)
            {
                for (float x = -radius + PIXEL_SIZE * 0.5f; x < radius; x += PIXEL_SIZE)
                {
                    if ((x * x + y * y) <= (radius * radius))
                    {
                        pixels.push_back({center + glm::vec2(x, y), color});
                    }
                }
            }
            return pixels;
        };

        SDL_Window *gameWindow = SDL_CreateWindow(
            "Game Preview",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        SDL_GL_MakeCurrent(gameWindow, sdlInterface.getGLContext());
        _boxWorld.shutdown();
        _boxWorld.init({0.0f, -2500.0f});
        b2WorldId worldId = _boxWorld.getWorldId();

        const glm::vec2 cameraCenter = _camera.getPosition();
        const float cameraZoom = _camera.getZoom() > 0.0f ? _camera.getZoom() : 1.0f;
        const float previewHalfWidth = (WINDOW_WIDTH * 0.5f) / cameraZoom;
        const float previewHalfHeight = (WINDOW_HEIGHT * 0.5f) / cameraZoom;
        const float groundHalfThickness = 5.0f;
        const float groundCenterY = cameraCenter.y - previewHalfHeight + groundHalfThickness;

        b2BodyDef groundDef = b2DefaultBodyDef();
        groundDef.type = b2_staticBody;
        groundDef.position = {cameraCenter.x, groundCenterY};
        _groundBody = b2CreateBody(worldId, &groundDef);

        b2ShapeDef groundShapeDef = b2DefaultShapeDef();
        groundShapeDef.density = 0.0f;
        groundShapeDef.material.friction = 0.6f;
        const b2Polygon groundBox = b2MakeBox(previewHalfWidth, groundHalfThickness);
        b2CreatePolygonShape(_groundBody, &groundShapeDef, &groundBox);

        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> xDist(cameraCenter.x - previewHalfWidth * 0.75f, cameraCenter.x + previewHalfWidth * 0.75f);
        std::uniform_real_distribution<float> boxHalfExtentDist(14.0f, 28.0f);
        std::uniform_real_distribution<float> circleRadiusDist(12.0f, 24.0f);
        std::uniform_real_distribution<float> rotationDist(-0.9f, 0.9f);
        std::uniform_real_distribution<float> angularVelocityDist(-1.5f, 1.5f);
        std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);
        std::uniform_int_distribution<int> shapeTypeDist(0, 1);

        std::vector<DemoShape> demoShapes;
        demoShapes.reserve(15);

        for (int index = 0; index < 15; ++index)
        {
            DemoShape shape;
            shape.type = shapeTypeDist(rng) == 0 ? DemoShapeType::Box : DemoShapeType::Circle;
            shape.color = {colorDist(rng), colorDist(rng), colorDist(rng)};

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_dynamicBody;
            bodyDef.position = {xDist(rng), cameraCenter.y + previewHalfHeight + 80.0f + static_cast<float>(index) * 70.0f};
            bodyDef.rotation = b2MakeRot(rotationDist(rng));
            bodyDef.angularVelocity = angularVelocityDist(rng);
            bodyDef.fixedRotation = false;
            shape.bodyId = b2CreateBody(worldId, &bodyDef);

            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.density = 1.0f;
            shapeDef.material.friction = 0.4f;
            shapeDef.material.restitution = 0.1f;

            if (shape.type == DemoShapeType::Box)
            {
                const float halfW = boxHalfExtentDist(rng);
                const float halfH = boxHalfExtentDist(rng);
                shape.size = {halfW * 2.0f, halfH * 2.0f};

                const b2Polygon box = b2MakeBox(halfW, halfH);
                b2CreatePolygonShape(shape.bodyId, &shapeDef, &box);
            }
            else
            {
                shape.radius = circleRadiusDist(rng);

                b2Circle circle;
                circle.center = {0.0f, 0.0f};
                circle.radius = shape.radius;
                b2CreateCircleShape(shape.bodyId, &shapeDef, &circle);
            }

            demoShapes.push_back(shape);
        }

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

            _pixelSimulation.update();
            _boxWorld.step(1.0f / 60.0f, 4);

            renderer.clear();

            std::vector<graphics::Pixel> framePixels = buildRenderPixels(_pixelSimulation.getGrid());

            for (const DemoShape& shape : demoShapes)
            {
                const b2Transform bodyTransform = b2Body_GetTransform(shape.bodyId);
                const b2Vec2 bodyPosition = bodyTransform.p;
                const float bodyRotation = b2Rot_GetAngle(bodyTransform.q);

                if (shape.type == DemoShapeType::Box)
                {
                    std::vector<graphics::Pixel> shapePixels = buildRotatedSquarePixels(
                        {bodyPosition.x, bodyPosition.y},
                        glm::max(shape.size.x, shape.size.y),
                        bodyRotation,
                        shape.color);
                    framePixels.insert(framePixels.end(), shapePixels.begin(), shapePixels.end());
                }
                else
                {
                    std::vector<graphics::Pixel> shapePixels = buildCirclePixels(
                        {bodyPosition.x, bodyPosition.y},
                        shape.radius,
                        shape.color);
                    framePixels.insert(framePixels.end(), shapePixels.begin(), shapePixels.end());
                }
            }

            std::vector<graphics::Pixel> groundPixels = buildRectanglePixels(
                {cameraCenter.x, groundCenterY},
                previewHalfWidth * 2.0f,
                groundHalfThickness * 2.0f,
                {0.2f, 0.2f, 0.2f});
            framePixels.insert(framePixels.end(), groundPixels.begin(), groundPixels.end());

            _renderPixels = framePixels;
            renderer.drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);

            renderer.present(gameWindow);
        }

        _boxWorld.shutdown();
        _cubeBody = b2_nullBodyId;
        _groundBody = b2_nullBodyId;

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

    std::vector<graphics::Pixel> Core::buildSquarePixels(glm::vec2 center, float size, glm::vec3 color) const
    {
        std::vector<graphics::Pixel> pixels;
        const float halfSize = size * 0.5f;

        for (float y = -halfSize + PIXEL_SIZE * 0.5f; y < halfSize; y += PIXEL_SIZE)
        {
            for (float x = -halfSize + PIXEL_SIZE * 0.5f; x < halfSize; x += PIXEL_SIZE)
            {
                pixels.push_back({center + glm::vec2(x, y), color});
            }
        }

        return pixels;
    }

    std::vector<graphics::Pixel> Core::buildRotatedSquarePixels(glm::vec2 center, float size, float rotation, glm::vec3 color) const
    {
        std::vector<graphics::Pixel> pixels;
        const float halfSize = size * 0.5f;
        const float cosine = std::cos(rotation);
        const float sine = std::sin(rotation);

        for (float y = -halfSize + PIXEL_SIZE * 0.5f; y < halfSize; y += PIXEL_SIZE)
        {
            for (float x = -halfSize + PIXEL_SIZE * 0.5f; x < halfSize; x += PIXEL_SIZE)
            {
                const float rotatedX = x * cosine - y * sine;
                const float rotatedY = x * sine + y * cosine;
                pixels.push_back({center + glm::vec2(rotatedX, rotatedY), color});
            }
        }

        return pixels;
    }

    std::vector<graphics::Pixel> Core::buildRectanglePixels(glm::vec2 center, float width, float height, glm::vec3 color) const
    {
        std::vector<graphics::Pixel> pixels;
        const float halfWidth = width * 0.5f;
        const float halfHeight = height * 0.5f;

        for (float y = -halfHeight + PIXEL_SIZE * 0.5f; y < halfHeight; y += PIXEL_SIZE)
        {
            for (float x = -halfWidth + PIXEL_SIZE * 0.5f; x < halfWidth; x += PIXEL_SIZE)
            {
                pixels.push_back({center + glm::vec2(x, y), color});
            }
        }

        return pixels;
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
                isGamePreviewActive = true;
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

        // ECS can stay variable
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
        if (projectEditor)
        {
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
        _pixelSimulation.setGrid(_chunkGrid);
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

            // loop on components in game object and add to ECS entity
            for (const auto &[compType, compData] : go.components)
            {
                if (compType == std::type_index(typeid(ecs::components::Transform)))
                {
                    const auto &t = std::any_cast<ecs::components::Transform>(compData);
                    componentManager.addComponent(eid, t);
                }
                else if (compType == std::type_index(typeid(ecs::components::Velocity)))
                {
                    const auto &v = std::any_cast<ecs::components::Velocity>(compData);
                    componentManager.addComponent(eid, v);
                }
                else if (compType == std::type_index(typeid(ecs::components::Sprite)))
                {
                    const auto &s = std::any_cast<ecs::components::Sprite>(compData);
                    componentManager.addComponent(eid, s);
                }
            }

            _gameObjectToEntity[go.id] = eid;
        }
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

        for (const auto &[key, chunk] : grid.chunks)
        {
            // Correct signed decode from packed int64 key
            const int cx = static_cast<int32_t>(key >> 32);
            const int cy = static_cast<int32_t>(key & 0xFFFFFFFF);

            for (int y = 0; y < CHUNK_SIZE; ++y)
            {
                for (int x = 0; x < CHUNK_SIZE; ++x)
                {
                    const Element::Pixel &simPixel = chunk.pixels[y * CHUNK_SIZE + x];
                    if (simPixel.type == Element::EMPTY)
                        continue;

                    const auto &def = g_elements[simPixel.type];

                    graphics::Pixel renderPixel;

                    // grid -> world (apply chunk offset + pixel size)
                    const float gx = static_cast<float>(cx * CHUNK_SIZE + x);
                    const float gy = static_cast<float>(cy * CHUNK_SIZE + y);

                    renderPixel.position = glm::vec2(
                        gx * PIXEL_SIZE,
                        gy * PIXEL_SIZE);

                    renderPixel.color = glm::vec3(
                        def.color[0] / 255.0f,
                        def.color[1] / 255.0f,
                        def.color[2] / 255.0f);

                    result.push_back(renderPixel);
                }
            }
        }

        return result;
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