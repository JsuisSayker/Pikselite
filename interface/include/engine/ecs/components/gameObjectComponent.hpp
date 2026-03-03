#pragma once

#include <vector>
#include <engine/pixels/pixelEnum.hpp>

namespace ecs::components
{
    // Links an ECS entity back to a Pixel::GameObject
    struct GameObjectLink
    {
        Pixel::GameObjectID gameObjectId = Pixel::NO_SPRITE;
        std::vector<Pixel::PixelEntityID> pixelEntities;
    };
} // namespace ecs::components
