#pragma once

#include "Ievent.hpp"

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <cstdint>
#include <algorithm>

namespace engine::events
{
    class EventBus
    {
    public:
        using HandlerId = std::uint64_t;

        template <typename EventT>
        HandlerId subscribe(std::function<void(const EventT &)> handler)
        {
            auto type = std::type_index(typeid(EventT));
            HandlerId id = ++lastId;
            auto wrapper = [h = std::move(handler)](const IEvent &base)
            {
                auto derived = dynamic_cast<const EventT *>(&base);
                if (derived)
                    h(*derived);
            };
            handlers[type].push_back({id, std::move(wrapper)});
            return id;
        }

        template <typename EventT>
        void unsubscribe(HandlerId id)
        {
            auto type = std::type_index(typeid(EventT));
            auto it = handlers.find(type);
            if (it == handlers.end())
                return;
            auto &vec = it->second;
            vec.erase(std::remove_if(vec.begin(), vec.end(), [id](const Handler &h)
                                     { return h.id == id; }),
                      vec.end());
        }

        // publish takes ownership of the event via unique_ptr
        void publish(std::unique_ptr<IEvent> ev);

    private:
        struct Handler
        {
            HandlerId id;
            std::function<void(const IEvent &)> fn;
        };
        std::unordered_map<std::type_index, std::vector<Handler>> handlers;
        HandlerId lastId = 0;
    };

} // namespace engine::events
