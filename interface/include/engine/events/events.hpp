#pragma once

#include "Ievent.hpp"

namespace engine::events
{
    /**
     * @brief The QuitEvent is an event that signals the application to quit.
     *
     */
    struct QuitEvent : IEvent
    {
        int code = 0;
    };
} // namespace engine::events
