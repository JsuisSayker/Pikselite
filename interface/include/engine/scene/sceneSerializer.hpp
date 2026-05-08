#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <engine/pixels/pixelEnum.hpp>

namespace engine::scene
{
    struct SceneData
    {
        std::vector<Pixel::GameObject> gameObjects;
        uint32_t nextGameObjectId = 1;
    };

    bool saveSceneToFile(const std::string& filename, const SceneData& data);
    bool loadSceneFromFile(const std::string& filename, SceneData& outData);
}
