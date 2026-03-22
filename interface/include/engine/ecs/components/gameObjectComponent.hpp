#pragma once

#include <vector>
#include <engine/pixels/pixelEnum.hpp>

#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/pixels/simulation/element/pixel.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>

namespace ecs::components
{
    // Links an ECS entity back to a Pixel::GameObject
    struct GameObjectLink
    {
        Pixel::GameObjectID gameObjectId = Pixel::NO_SPRITE;
        std::vector<Element::Pixel> pixelEntities;
    };

    
} // namespace ecs::components
