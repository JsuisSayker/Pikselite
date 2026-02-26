#pragma once

namespace ecs::components
{
    struct Transform {
        float x, y;
        float rotation;
        float scaleX, scaleY;

        // Previous frame position, used to compute movement delta
        float prevX = 0.0f, prevY = 0.0f;
    };
} // namespace ecs::components
