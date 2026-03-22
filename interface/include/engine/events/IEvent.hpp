#pragma once


namespace engine::events
{
    /**
     * @brief The IEvent interface defines the basic structure for all events in the application.
     */
    struct IEvent
    {
        virtual ~IEvent() = default;
    };
} // namespace events