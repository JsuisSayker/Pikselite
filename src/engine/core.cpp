#include <atomic>
#include <cmath>
#include <cstdio> // for _popen, _pclose
#include <engine/core.hpp>
#include <engine/ecs/systems/spriteRenderSystem.hpp>
#include <engine/scene/sceneSerializer.hpp>
#include <fstream>
#include <mutex>
#include <thread>
#include <tracy/Tracy.hpp>
#include <unordered_set>

#include <cmath>
#include <algorithm>
#include <cstdint>

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

    graphics::Pixel pixelFromJson(const json& j)
    {
        return {{j.value("x", 0.0f), j.value("y", 0.0f)},
                {j.value("r", 1.0f), j.value("g", 1.0f), j.value("b", 1.0f)}};
    }

    json gameObjectToJson(const Pixel::GameObject& go)
    {
        // TODO
    }

    Pixel::GameObject gameObjectFromJson(const json& j)
    {
        // TODO
    }

} // namespace

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
            }
            else if (isProjectsListPageActive)
            {
                ZoneScopedN("ProjectsListPage");
                runProjectsListPage(sdlInterface, renderer, imguiInterface);
            }
            else if (isProjectEditorActive)
            {
                ZoneScopedN("ProjectEditor");

                // Handle build request (before frame)
                if (projectEditor->consumeBuildGameRequest())
                {
                    BuildSettings settings;
                    settings.gameTitle  = _currentProject.name;
                    settings.targetName = _currentProject.name;
                    settings.scenePath  = _sceneFilename;
                    projectEditor->showBuildSettings(settings);
                }

                // Snapshot build progress state (before frame)
                {
                    std::string snapshot;
                    {
                        std::lock_guard<std::mutex> lock(_buildOutputMutex);
                        snapshot = _buildOutput;
                    }
                    projectEditor->setBuildProgress(_isBuilding, _buildDone, _buildSuccess,
                                                    snapshot);
                }

                // Render frame (ImGui rendering inside ProjectEditor::run)
                projectEditor->run(event);

                // Handle build confirmed (after frame — start the build thread)
                BuildSettings confirmedSettings;
                if (projectEditor->consumeBuildConfirmed(confirmedSettings))
                {
                    // Snapshot editor data before spawning thread
                    copyProjectEditorDataToCore();

                    const std::string& targetName = confirmedSettings.targetName;
                    const std::string  assetsDir  = "games/" + targetName + "/assets";

                    // Save scene
                    std::filesystem::create_directories(assetsDir);
                    const std::string scenePath = assetsDir + "/scene.scene";
                    saveScene(scenePath);

                    // Collect used .dat files
                    std::vector<std::string> neededDats;
                    for (const auto& go : _gameObjects)
                    {
                        if (!go.sourceDatPath.empty())
                            neededDats.push_back(go.sourceDatPath);
                    }

                    if (_buildThread.joinable())
                        _buildThread.join();

                    _isBuilding   = true;
                    _buildDone    = false;
                    _buildSuccess = false;
                    {
                        std::lock_guard<std::mutex> lock(_buildOutputMutex);
                        _buildOutput.clear();
                    }
                    _buildThread = std::thread(
                        [this, confirmedSettings, neededDats]()
                        {
                            bool success = buildGame(confirmedSettings, neededDats);
                            {
                                std::lock_guard<std::mutex> lock(_buildOutputMutex);
                                _buildOutput +=
                                    success ? "Build completed successfully.\n" : "Build failed.\n";
                            }
                            _buildSuccess = success;
                            _buildDone    = true;
                        });
                }

                // Handle build progress dismissed
                if (projectEditor->consumeBuildProgressDismissed())
                {
                    if (_buildThread.joinable())
                        _buildThread.join();
                    _isBuilding = false;
                }

                // Save/load scene
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
            }
            else if (isSpriteEditorActive)
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
            drawSpritesBelowLayer(0);
            renderer.drawPixelsWCamera(framePixels, _camera, PIXEL_SIZE);
            renderer.drawParticlesWCamera(_pixelSimulation.getParticles(), _camera, PIXEL_SIZE);
            drawSpritesAboveLayer(0);

            // debug draw Box2D bodies
            if (auto* physicsSystem = systemManager.getSystem<ecs::systems::PhysicsSystem>())
            {
                const std::vector<b2BodyId> ecsDebugBodies = physicsSystem->getDebugBodies();
                renderer.drawBox2DDebug(_boxWorld.getWorldId(), ecsDebugBodies, _camera, 1.0f,
                                        glm::vec3(1.0f, 0.8f, 0.2f));
            }

            

            // if (auto *spriteSystem = systemManager.getSystem<ecs::systems::SpriteRenderSystem>())
            // {
            //     spriteSystem->update(0.0, componentManager);
            // }

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
                  { return a.lastOpened > b.lastOpened; });
    }

    void Core::openProject(int index)
    {
        _currentProject = _projects[index];

        auto now                    = std::chrono::system_clock::now();
        _currentProject.lastOpened  = now;
        _projects[index].lastOpened = now;

        sortProjects(_projects);
        saveProjects(_projects);

        switchToProjectEditor = true;
    }

    void Core::runProjectsListPage(graphics::Interface& sdlInterface, graphics::Renderer& renderer,
                                   graphics::ImguiInterface& imguiInterface)
    {
        renderer.clear();
        imguiInterface.startFrame();

        imguiInterface.fileToolBar();

        float toolbarHeight = 40.0f;

        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, toolbarHeight));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - toolbarHeight));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("MainWindow", nullptr, flags);

        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(170, 100));
        ImGui::BeginChild("projectOptions", ImVec2(0, 250), true,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        int selectedProjectIndex = imguiInterface.projectOptionsBar(_projects, _projectsPath);
        if (selectedProjectIndex >= 0 && selectedProjectIndex < _projects.size())
        {
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

            if (selectedProjectIndex >= 0 && selectedProjectIndex < _projects.size())
            {
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

        if (switchToProjectEditor)
        {
            isProjectsListPageActive = false;
            isProjectEditorActive    = true;
            switchToProjectEditor    = false;
            projectEditor->setCurrentProject(_currentProject);
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
                    isSpriteEditorActive  = !isSpriteEditorActive;
                }
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
                ZoneScopedN("Sim::Tick");
                _pixelSimulation.update();
                syncRegionBodiesToECS();

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

    void Core::syncRegionBodiesToECS()
    {
        for (ecs::EntityID entityId : _regionBodyEntities)
        {
            componentManager.entityDestroyed(entityId);
            systemManager.entityDestroyed(entityId);
            entityManager.destroyEntity(ecs::Entity(entityId));
        }
        _regionBodyEntities.clear();

        const std::vector<b2BodyId> regionBodies = _pixelSimulation.getRegionBodies();
        if (regionBodies.empty())
        {
            return;
        }

        _regionBodyEntities.reserve(regionBodies.size());

        for (const b2BodyId bodyId : regionBodies)
        {
            if (!b2Body_IsValid(bodyId))
            {
                continue;
            }

            const b2Transform transform = b2Body_GetTransform(bodyId);

            ecs::Entity entity = entityManager.createEntity();
            const ecs::EntityID entityId = entity.id;

            ecs::components::Transform transformComponent{};
            transformComponent.x = transform.p.x;
            transformComponent.y = transform.p.y;
            transformComponent.rotation = b2Rot_GetAngle(transform.q);
            transformComponent.scaleX = 1.0f;
            transformComponent.scaleY = 1.0f;

            ecs::components::PhysicsBody physicsComponent{};
            physicsComponent.bodyId = bodyId;
            physicsComponent.bodyType = b2_staticBody;
            physicsComponent.fixedRotation = true;

            componentManager.addComponent(entityId, transformComponent);
            componentManager.addComponent(entityId, physicsComponent);

            b2Body_SetUserData(bodyId, reinterpret_cast<void*>(static_cast<std::uintptr_t>(entityId)));

            _regionBodyEntities.push_back(entityId);
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
        }
        if (_buildThread.joinable())
        {
            _buildDone = true;
            _buildThread.join();
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

    bool Core::buildGame(const BuildSettings& settings, const std::vector<std::string>& neededDats)
    {
        const std::string targetName = settings.targetName;
        const std::string gameDir    = "games/" + targetName;
        const std::string srcDir     = gameDir + "/src";
        const std::string assetsDir  = gameDir + "/assets";

        auto appendOutput = [this](const std::string& msg)
        {
            std::lock_guard<std::mutex> lock(_buildOutputMutex);
            _buildOutput += msg + "\n";
        };

        // Create directories
        appendOutput("Creating game directory: " + gameDir);
        std::filesystem::create_directories(srcDir);
        std::filesystem::create_directories(assetsDir);

        // Generate CMakeLists.txt
        {
            const std::string cmakeContent =
                "# -------------------------------------------------\n"
                "# " +
                targetName +
                " - Auto-generated game project\n"
                "# -------------------------------------------------\n"
                "add_executable(" +
                targetName +
                " src/main.cpp)\n"
                "\n"
                "target_include_directories(" +
                targetName +
                "\n"
                "    PRIVATE\n"
                "        ${CMAKE_SOURCE_DIR}/src\n"
                "        ${CMAKE_SOURCE_DIR}/interface/include\n"
                ")\n"
                "\n"
                "target_link_libraries(" +
                targetName +
                "\n"
                "    PRIVATE\n"
                "        engine\n"
                "        graphics\n"
                "        game\n"
                "        pikselite_interface\n"
                "        SDL2::SDL2\n"
                "        SDL2::SDL2main\n"
                "        GLEW::GLEW\n"
                "        OpenGL::GL\n"
                "        box2d::box2d\n"
                "        ZLIB::ZLIB\n"
                "        ${LUA_LIBRARIES}\n"
                "        nlohmann_json::nlohmann_json\n"
                ")\n"
                "\n"
                "# Copy assets to output directory\n"
                "add_custom_command(TARGET " +
                targetName +
                " POST_BUILD\n"
                "    COMMAND ${CMAKE_COMMAND} -E copy_directory\n"
                "        ${CMAKE_CURRENT_SOURCE_DIR}/assets\n"
                "        $<TARGET_FILE_DIR:" +
                targetName +
                ">/assets\n"
                "    COMMENT \"Copying game assets to output directory\"\n"
                ")\n";

            std::ofstream cmakeFile(gameDir + "/CMakeLists.txt");
            if (!cmakeFile)
            {
                appendOutput("ERROR: Failed to create CMakeLists.txt");
                return false;
            }
            cmakeFile << cmakeContent;
            appendOutput("Generated CMakeLists.txt");
        }

        // Generate main.cpp
        {
            const std::string mainContent =
                "#define SDL_MAIN_HANDLED\n"
                "#include <game/Game.hpp>\n"
                "#include <filesystem>\n"
                "\n"
                "int main(int argc, char* argv[])\n"
                "{\n"
                "    std::filesystem::current_path(\n"
                "        std::filesystem::absolute(argv[0]).parent_path());\n"
                "    engine::Game game(" +
                std::to_string(settings.windowWidth) + ", " +
                std::to_string(settings.windowHeight) + ", \"" + settings.gameTitle +
                "\");\n"
                "    if (!game.loadScene(\"assets/scene.scene\"))\n"
                "        return 1;\n"
                "    game.run();\n"
                "    return 0;\n"
                "}\n";

            std::ofstream mainFile(srcDir + "/main.cpp");
            if (!mainFile)
            {
                appendOutput("ERROR: Failed to create main.cpp");
                return false;
            }
            mainFile << mainContent;
            appendOutput("Generated main.cpp");
        }

        // Copy .dat files used by the scene
        for (const auto& datPath : neededDats)
        {
            std::filesystem::path src = datPath;
            std::filesystem::path dst = std::filesystem::path(assetsDir) / src.filename();
            std::error_code       ec;
            std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing,
                                       ec);
            if (ec)
            {
                appendOutput("WARNING: Failed to copy " + datPath + ": " + ec.message());
            }
            else
            {
                appendOutput("Copied " + src.filename().string());
            }
        }

        // Re-configure CMake to pick up the new target
        {
            appendOutput("Re-configuring CMake...");
            std::string configureCmd = "cmake -B build 2>&1";
            FILE*       pipe         = _popen(configureCmd.c_str(), "r");
            if (!pipe)
            {
                appendOutput("ERROR: Failed to run cmake configure");
                return false;
            }
            char buffer[128]; // needs to be reworked
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                std::string line(buffer);
                // Trim newline
                if (!line.empty() && line.back() == '\n')
                    line.pop_back();
                appendOutput("  " + line);
            }
            int exitCode = _pclose(pipe);
            if (exitCode != 0)
            {
                appendOutput("ERROR: CMake configure failed with exit code " +
                             std::to_string(exitCode));
                return false;
            }
            appendOutput("CMake configure succeeded.");
        }

        // Build the game target
        {
            appendOutput("Building target " + targetName + "...");
            std::string buildCmd =
                "cmake --build build --target " + targetName + " --config Release 2>&1";
            FILE* pipe = _popen(buildCmd.c_str(), "r");
            if (!pipe)
            {
                appendOutput("ERROR: Failed to run cmake build");
                return false;
            }
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                std::string line(buffer);
                if (!line.empty() && line.back() == '\n')
                    line.pop_back();
                appendOutput("  " + line);
            }
            int exitCode = _pclose(pipe);
            if (exitCode != 0)
            {
                appendOutput("ERROR: Build failed with exit code " + std::to_string(exitCode));
                return false;
            }
            appendOutput("Build succeeded.");
        }

        appendOutput("Game build complete! Output: build/" + targetName + "/Release/" + targetName +
                     ".exe");
        return true;
    }

    void Core::saveProjects(const std::vector<projects::Project>& projects)
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

            j.push_back({{"name", p.name}, {"path", p.path.string()}, {"lastOpened", t}});
        }

        file << j.dump(4);
    }

    void Core::getProjectsFolderPath()
    {
        std::filesystem::path exeDir       = std::filesystem::current_path();
        std::filesystem::path projectsPath = exeDir / "Projects";
        std::filesystem::path infoPath     = "config/info.json";

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

        _projectsPath    = projectsPath.string();
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
        std::filesystem::path infoPath  = configDir / "info.json";

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

        if (!j.contains("defaultPath") || j["defaultPath"].is_null() ||
            j["defaultPath"].get<std::string>().empty())
        {
            getProjectsFolderPath();
            return;
        }

        _projectsPath = j["defaultPath"].get<std::string>();
    }

    void Core::loadProjects(std::vector<projects::Project>& projects)
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

            auto fileTime   = std::filesystem::last_write_time(entry.path());
            auto systemTime = std::chrono::system_clock::now() +
                              (fileTime - std::filesystem::file_time_type::clock::now());

            project.lastOpened =
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(systemTime);
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

    void Core::drawSpritesBelowLayer(int layer)
    {
        struct SpriteDrawItem
        {
            int layer;
            ecs::EntityID entityId;
        };

        std::vector<SpriteDrawItem> drawList;
        const auto& entities = entityManager.getEntities();
        drawList.reserve(entities.size());

        for (const auto& entityPtr : entities)
        {
            if (!entityPtr) continue;
            const ecs::EntityID entityId = entityPtr->id;

            if (!componentManager.hasComponent<ecs::components::Transform>(entityId)) continue;
            if (!componentManager.hasComponent<ecs::components::Sprite>(entityId)) continue;

            auto& sprite = componentManager.getComponent<ecs::components::Sprite>(entityId);
            auto& transform = componentManager.getComponent<ecs::components::Transform>(entityId);

            if (!sprite.enabled || !transform.enabled) continue;
            if (sprite.layer >= layer) continue;

            drawList.push_back({sprite.layer, entityId});
        }

        std::sort(drawList.begin(), drawList.end(), [](const SpriteDrawItem& a, const SpriteDrawItem& b)
        {
            if (a.layer != b.layer) return a.layer < b.layer;
            return a.entityId < b.entityId;
        });

        for (const auto& item : drawList)
        {
            auto& sprite = componentManager.getComponent<ecs::components::Sprite>(item.entityId);
            auto& transform = componentManager.getComponent<ecs::components::Transform>(item.entityId);

            if (!sprite.loaded && !sprite.texturePath.empty())
            {
                sprite.textureID = renderer.loadTexture(sprite.texturePath);
                sprite.loaded = true;
            }

            if (sprite.textureID == 0) continue;

            graphics::Sprite2D s2d;
            s2d.position = {transform.x, transform.y};
            s2d.size = {sprite.width * transform.scaleX, sprite.height * transform.scaleY};
            s2d.textureID = sprite.textureID;

            renderer.drawSprite(s2d, _camera);
        }
    }

    void Core::drawSpritesAboveLayer(int layer)
    {
        struct SpriteDrawItem
        {
            int layer;
            ecs::EntityID entityId;
        };

        std::vector<SpriteDrawItem> drawList;
        const auto& entities = entityManager.getEntities();
        drawList.reserve(entities.size());

        for (const auto& entityPtr : entities)
        {
            if (!entityPtr) continue;
            const ecs::EntityID entityId = entityPtr->id;

            if (!componentManager.hasComponent<ecs::components::Transform>(entityId)) continue;
            if (!componentManager.hasComponent<ecs::components::Sprite>(entityId)) continue;

            auto& sprite = componentManager.getComponent<ecs::components::Sprite>(entityId);
            auto& transform = componentManager.getComponent<ecs::components::Transform>(entityId);

            if (!sprite.enabled || !transform.enabled) continue;
            if (sprite.layer <= layer) continue;

            drawList.push_back({sprite.layer, entityId});
        }

        std::sort(drawList.begin(), drawList.end(), [](const SpriteDrawItem& a, const SpriteDrawItem& b)
        {
            if (a.layer != b.layer) return a.layer < b.layer;
            return a.entityId < b.entityId;
        });

        for (const auto& item : drawList)
        {
            auto& sprite = componentManager.getComponent<ecs::components::Sprite>(item.entityId);
            auto& transform = componentManager.getComponent<ecs::components::Transform>(item.entityId);

            if (!sprite.loaded && !sprite.texturePath.empty())
            {
                sprite.textureID = renderer.loadTexture(sprite.texturePath);
                sprite.loaded = true;
            }

            if (sprite.textureID == 0) continue;

            graphics::Sprite2D s2d;
            s2d.position = {transform.x, transform.y};
            s2d.size = {sprite.width * transform.scaleX, sprite.height * transform.scaleY};
            s2d.textureID = sprite.textureID;

            renderer.drawSprite(s2d, _camera);
        }
    }
} // namespace engine