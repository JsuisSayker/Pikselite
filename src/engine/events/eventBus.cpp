#include "engine/events/eventBus.hpp"

namespace engine::events
{
    void EventBus::publish(std::unique_ptr<IEvent> ev)
    {
        if (!ev)
            return;
        auto type = std::type_index(typeid(*ev));
        auto it = handlers.find(type);
        if (it == handlers.end())
            return;
        const IEvent& ref = *ev;
        for (auto& h : it->second)
            h.fn(ref);
    }

} // namespace engine::events
