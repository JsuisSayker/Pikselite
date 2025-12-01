#pragma once


namespace engine::events
{
    struct IEvent
    {
        virtual ~IEvent() = default;
    };
} // namespace events