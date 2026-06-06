#pragma once

#include <box2d/box2d.h>
#include <graphics/graphicsEnum.hpp>
#include <vector>

namespace ecs::components
{
    /**
     * @brief Triangle primitive used to build polygon colliders for a physics body.
     */
    struct PhysicsTriangle
    {
        glm::vec2 a{0.0f, 0.0f};
        glm::vec2 b{0.0f, 0.0f};
        glm::vec2 c{0.0f, 0.0f};
    };

    /**
     * @brief Box2D-backed body description used by PhysicsSystem.
     *
     * When `triangles` is empty, PhysicsSystem creates a fallback box collider
     * from the sprite size (or pixel size default).
     */
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