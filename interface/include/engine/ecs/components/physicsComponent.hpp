#pragma once

#include <vector>

#include <box2d/box2d.h>
#include <graphics/graphicsEnum.hpp>

namespace ecs::components
{
    struct PhysicsTriangle
    {
        glm::vec2 a{0.0f, 0.0f};
        glm::vec2 b{0.0f, 0.0f};
        glm::vec2 c{0.0f, 0.0f};
    };

    struct PhysicsBody
    {
        bool enabled = true;
        b2BodyId bodyId = b2_nullBodyId;
        b2BodyType bodyType = b2_dynamicBody;
        bool fixedRotation = false;
        float density = 1.0f;
        float friction = 0.4f;
        float restitution = 0.1f;
        std::vector<PhysicsTriangle> triangles;
    };
} // namespace ecs::components