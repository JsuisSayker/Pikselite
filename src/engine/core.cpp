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
            } else if (isProjectsListPageActive) {
                ZoneScopedN("ProjectsListPage");
                runProjectsListPage(sdlInterface, renderer, imguiInterface);
            } else if (isProjectEditorActive)
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
            } else if (isSpriteEditorActive)
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

            std::vector<graphics::Pixel> framePixels = buildRenderPixels(_pixelSimulation.getGrid());
            _renderPixels = framePixels; // cache for potential editing after preview

            renderer.drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);

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

    void Core::runProjectsListPage(graphics::Interface& sdlInterface, graphics::Renderer& renderer, graphics::ImguiInterface& imguiInterface) ///////////////////////////
    {
        renderer.clear();
        imguiInterface.startFrame();

        imguiInterface.fileToolBar();

        float toolbarHeight = 40.0f;
        
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, toolbarHeight));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - toolbarHeight));

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("MainWindow", nullptr, flags);

        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(100, 50));
        ImGui::BeginChild("projectOptions", ImVec2(0, 150), true);
        imguiInterface.projectOptionsBar();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);

        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(100, 0));
        ImGui::BeginChild("projectDisplaySection", ImVec2(0, 0), true);
        imguiInterface.recentProjectsDisplay();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleVar();
        
        imguiInterface.endFrame(sdlInterface.getWindow());
        renderer.present(sdlInterface.getWindow());

        // Section (horizontal) with: Recent projects text, 2 formatting options b, List of the last 2 projects opened
        // The project display will show: thumbnail, Title of project, path to project, Date of last opened
        // Clicking on a project will: make isProjectEditorActive = false, isProjectEditorActive = true, and load the project data into the core
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
            if (!isProjectsListPageActive)
            {
                isProjectEditorActive = !isProjectEditorActive;
                isSpriteEditorActive = !isSpriteEditorActive;
            }
            break;
        case graphics::KEY_F5:
            if (!isGamePreviewActive)
            {
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

            ecs::components::Transform transform{0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f};
            componentManager.addComponent<ecs::components::Transform>(eid, transform);

            ecs::components::Velocity velocity{20.0f, 0.0f};
            componentManager.addComponent<ecs::components::Velocity>(eid, velocity);

            ecs::components::GameObjectLink link;
            link.gameObjectId = go.id;
            link.pixelEntities = go.pixels;
            componentManager.addComponent<ecs::components::GameObjectLink>(eid, link);

            ecs::components::Sprite spriteComp;
            spriteComp.texturePath = "assets/dragon.png";
            componentManager.addComponent<ecs::components::Sprite>(eid, spriteComp);

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