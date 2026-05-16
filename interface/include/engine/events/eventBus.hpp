#pragma once

#include "Ievent.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace engine::events
{
    /**
     * @brief The EventBus class manages the subscription and publishing of events.
     */
    class EventBus
    {
      public:
        using HandlerId = std::uint64_t;

        /**
         * @brief Subscribes to an event type.
         *
         * @tparam EventT
         * @param handler
         * @return HandlerId
         */
        template <typename EventT> HandlerId subscribe(std::function<void(const EventT&)> handler)
        {
            auto type = std::type_index(typeid(EventT));
            HandlerId id = ++lastId;
            auto wrapper = [h = std::move(handler)](const IEvent& base)
            {
                auto derived = dynamic_cast<const EventT*>(&base);
                if (derived)
                    h(*derived);
            };
            handlers[type].push_back({id, std::move(wrapper)});
            return id;
        }

        /**
         * @brief Unsubscribes from an event type using the handler ID returned by the subscribe
         * function.
         *
         * @tparam EventT
         * @param id
         */
        template <typename EventT> void unsubscribe(HandlerId id)
        {
            auto type = std::type_index(typeid(EventT));
            auto it = handlers.find(type);
            if (it == handlers.end())
                return;
            auto& vec = it->second;
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                                     [id](const Handler& h) { return h.id == id; }),
                      vec.end());
        }

        /**
         * @brief Publishes an event to all subscribed handlers.
         *
         * @param ev
         */
        void publish(std::unique_ptr<IEvent> ev);

      private:
        // Internal struct to store handler information
        struct Handler
        {
            HandlerId id;
            std::function<void(const IEvent&)> fn;
        };

        // Map of event type to list of handlers
        std::unordered_map<std::type_index, std::vector<Handler>> handlers;

        // Incremental ID generator for handlers
        HandlerId lastId = 0;
    };

} // namespace engine::events
