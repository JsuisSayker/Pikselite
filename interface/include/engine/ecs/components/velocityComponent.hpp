#pragma once

namespace ecs::components
{
    struct Velocity {
        bool enabled = true;
        
        float vx, vy;
    };
} // namespace ecs::components