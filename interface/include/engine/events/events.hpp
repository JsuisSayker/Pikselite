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

    /**
     * @brief Requests a scene reload/load.
     *
     * Published synchronously (e.g. from Lua's `reload_scene()` / `load_scene(path)`).
     * Handlers should *queue* the request and process it between frames, never
     * call `loadScene` directly from the handler — entities are likely being
     * iterated when this fires.
     *
     * An empty `path` means "reload the currently loaded scene".
     */
    struct SceneLoadRequestedEvent : IEvent
    {
        std::string path;
    };
} // namespace engine::events