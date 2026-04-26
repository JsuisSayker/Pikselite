#pragma once

#include "Ievent.hpp"
#include "engine/ecs/entity.hpp"

#include <string>

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

    struct CollisionEnterEvent : IEvent
    {
        ecs::EntityID entityA = 0;
        ecs::EntityID entityB = 0;
    };

    struct CollisionExitEvent : IEvent
    {
        ecs::EntityID entityA = 0;
        ecs::EntityID entityB = 0;
    };

    struct VictoryEvent : IEvent
    {
        std::string reason;
    };

    struct LoseEvent : IEvent
    {
        std::string reason;
    };
} // namespace events
