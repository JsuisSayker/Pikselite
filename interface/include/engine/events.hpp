#pragma once

#include "Ievent.hpp"

namespace engine::events
{
    struct QuitEvent : IEvent
    {
        int code = 0;
    };
} // namespace events
