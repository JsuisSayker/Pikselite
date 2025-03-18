#pragma once

namespace engine
{
    enum class Events : uint8_t
    {
        MOVE_UP,
        MOVE_DOWN,
        MOVE_LEFT,
        MOVE_RIGHT,
        JUMP,
        NONE
    };
} // namespace engine