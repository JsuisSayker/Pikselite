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
            }

            if (isProjectEditorActive)
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


} // namespace engine