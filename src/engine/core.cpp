#include <cmath>
#include <engine/core.hpp>
#include <tracy/Tracy.hpp>
#include <engine/ecs/systems/spriteRenderSystem.hpp>

#ifndef TRACY_ENABLE
// output a warning if profiling is disabled
#pragma message(                                                                                   \
    "Tracy profiling is disabled. To enable, set PIKSELITE_ENABLE_PROFILING=ON in CMake and rebuild.")
#error "Not set"
#endif
#include <box2d/box2d.h>
#include <engine/ecs/systems/physicsSystem.hpp>

namespace
{

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
                    _sceneFilename = projectEditor->getSceneFilename();
                    saveScene(_sceneFilename);
                }

                if (projectEditor->consumeLoadSceneRequest())
                {
                    _sceneFilename = projectEditor->getSceneFilename();
                    if (loadScene(_sceneFilename))
                    {
                        projectEditor->setSceneData(_renderPixels, _gameObjects, _chunkGrid,
                                                    gameObjectCounter);
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

    void Core::sortProjects(std::vector<projects::Project>& projects)
    {
        std::sort(projects.begin(), projects.end(),
            [](const projects::Project& a, const projects::Project& b)
            {
                return a.lastOpened > b.lastOpened;
            });
    }

    void Core::openProject(int index)
    {
        _currentProject = _projects[index];

        auto now = std::chrono::system_clock::now();
        _currentProject.lastOpened = now;
        _projects[index].lastOpened = now;

        sortProjects(_projects);
        saveProjects(_projects);

        switchToProjectEditor = true;
    }

    void Core::runProjectsListPage(graphics::Interface& sdlInterface, graphics::Renderer& renderer, graphics::ImguiInterface& imguiInterface)
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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(170, 100));
        ImGui::BeginChild("projectOptions", ImVec2(0, 250), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        int selectedProjectIndex = imguiInterface.projectOptionsBar(_projects, _projectsPath);
        if (selectedProjectIndex >= 0 && selectedProjectIndex < _projects.size()) {
            openProject(selectedProjectIndex);
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);

        float remainingHeight = ImGui::GetContentRegionAvail().y;

        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(150, 30));
        ImGui::BeginChild("projectDisplaySection", ImVec2(0, remainingHeight), true);
        if (!switchToProjectEditor)
        {
            selectedProjectIndex = imguiInterface.projectsDisplay(_projects);
        
            if (selectedProjectIndex >= 0 && selectedProjectIndex < _projects.size()) {
                openProject(selectedProjectIndex);
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleVar();
        
        imguiInterface.endFrame(sdlInterface.getWindow());
        renderer.present(sdlInterface.getWindow());

        if (switchToProjectEditor) {
            isProjectsListPageActive = false;
            isProjectEditorActive = true;
            switchToProjectEditor = false;
            projectEditor->setCurrentProject(_currentProject);
            spriteEditor->setProjectAssetsPath(_currentProject.path);
        }
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
        }
        _boxWorld.shutdown();
        SDL_Quit();
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

    void Core::saveProjects(const std::vector<projects::Project> &projects)
    {
        std::filesystem::create_directories("config");

        std::ofstream file("config/projects.json");

        if (!file.is_open())
        {
            std::cerr << "Failed to open projects.json for writing\n";
            return;
        }

        nlohmann::json j = nlohmann::json::array();

        for (const auto& p : projects)
        {
            std::time_t t = std::chrono::system_clock::to_time_t(p.lastOpened);

            j.push_back({
                {"name", p.name},
                {"path", p.path.string()},
                {"lastOpened", t}
            });
        }

        file << j.dump(4);
    }

    void Core::getProjectsFolderPath()
    {
        std::filesystem::path exeDir = std::filesystem::current_path();
        std::filesystem::path projectsPath = exeDir / "Projects";
        std::filesystem::path infoPath = "config/info.json";

        json j;

        if (std::filesystem::exists(infoPath))
        {
            std::ifstream inFile(infoPath);

            if (inFile.is_open())
            {
                inFile >> j;
                inFile.close();
            }
        }

        _projectsPath = projectsPath.string();
        j["defaultPath"] = _projectsPath;

        std::ofstream outFile(infoPath);

        if (outFile.is_open())
        {
            outFile << j.dump(4);
            outFile.close();
        }
    }

    void Core::getJsonVariables()
    {
        std::filesystem::path configDir = "config";
        std::filesystem::path infoPath = configDir / "info.json";

        if (!std::filesystem::exists(configDir))
        {
            std::filesystem::create_directories(configDir);
        }

        if (!std::filesystem::exists(infoPath))
        {
            json j;

            j["defaultPath"] = "";

            std::ofstream outFile(infoPath);

            if (outFile.is_open())
            {
                outFile << j.dump(4);
                outFile.close();
            }

            getProjectsFolderPath();
            return;
        }

        json j;

        std::ifstream inFile(infoPath);

        if (inFile.is_open())
        {
            inFile >> j;
            inFile.close();
        }

        if (!j.contains("defaultPath") ||
            j["defaultPath"].is_null() ||
            j["defaultPath"].get<std::string>().empty())
        {
            getProjectsFolderPath();
            return;
        }

        _projectsPath = j["defaultPath"].get<std::string>();
    }

    void Core::loadProjects(std::vector<projects::Project> &projects)
    {
        projects.clear();

        std::filesystem::path projectsPath = _projectsPath;

        if (!std::filesystem::exists(projectsPath))
        {
            std::filesystem::create_directories(projectsPath);
            return;
        }

        if (std::filesystem::is_empty(projectsPath))
        {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(projectsPath))
        {
            if (!entry.is_directory())
            {
                continue;
            }

            projects::Project project;

            auto fileTime = std::filesystem::last_write_time(entry.path());
            auto systemTime = std::chrono::system_clock::now() + (fileTime - std::filesystem::file_time_type::clock::now());

            project.lastOpened = std::chrono::time_point_cast<std::chrono::system_clock::duration>(systemTime);
            project.name = entry.path().filename().string();
            project.path = entry.path();

            projects.push_back(project);
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