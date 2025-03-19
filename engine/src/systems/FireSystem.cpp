#include <systems/FireSystem.hpp>

void FireSystem::update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events)
{
    if (events.empty())
        return;

    graphic::EventType event = events.back();

    for (auto &sprite : sprites)
    {
        auto it = sprite.actions.find(event);

        if (it == nullptr)
            return;

        // if (it->second == engine::Events::FIRE)
        // {
        //     graphic::Sprite bullet = sprite;
        //     bullet.position.y -= 10;
        //     bullet.position.x += 10;
        //     sprites.push_back(bullet);
        // }
    }
}