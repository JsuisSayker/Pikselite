#include <cmath>
#include <engine/core.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <engine/scene/sceneSerializer.hpp>
#include <filesystem>
#include <iostream>

namespace engine
{
    void Core::saveScene(const std::string& filename)
    {
        if (filename.empty())
            return;

        const std::filesystem::path scenePath(filename);
        if (!scenePath.parent_path().empty())
        {
            std::filesystem::create_directories(scenePath.parent_path());
        }

        engine::scene::SceneData data;
        data.gameObjects = _gameObjects;
        data.nextGameObjectId = gameObjectCounter;

        if (!engine::scene::saveSceneToFile(filename, data))
        {
            std::cerr << "Failed to save scene: " << filename << std::endl;
        }
    }

    bool Core::loadScene(const std::string& filename)
    {
        if (filename.empty())
            return false;

        engine::scene::SceneData data;
        if (!engine::scene::loadSceneFromFile(filename, data))
        {
            std::cerr << "Failed to load scene: " << filename << std::endl;
            return false;
        }

        _gameObjects = std::move(data.gameObjects);
        gameObjectCounter = data.nextGameObjectId;

        _chunkGrid.chunks.clear();
        _renderPixels.clear();
        _gameObjectToEntity.clear();
        _gameObjectOccupiedCells.clear();

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
                scenePixel.colorIndex = renderer.generatePixelColorIndex(gridX, gridY);
                scenePixel.isBurning = pixel.isBurning;
                // FIRE: restore burn timer from save; if zero (legacy/empty), seed from element
                // definition.
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

        _pixelSimulation.setGrid(_chunkGrid);
        _renderPixels = buildRenderPixels(_chunkGrid);
        loadGameObjectsIntoECS();
        return true;
    }
} // namespace engine
