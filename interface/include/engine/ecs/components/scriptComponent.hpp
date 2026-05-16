#pragma once

#include <string>

namespace ecs::components
{
    /**
     * @brief Attach a Lua script to an entity.
     */
    struct Script
    {
        bool enabled = true;
        std::string scriptPath = "scripts/movement.lua";
    };
} // namespace ecs::components
