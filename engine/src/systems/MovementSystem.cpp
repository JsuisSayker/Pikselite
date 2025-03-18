#include "systems/MovementSystem.hpp"

void MovementSystem::update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events)
{
    if (events.empty())
        return;

    graphic::EventType event = events.back();

    for (auto &sprite : sprites)
    {
        auto it = sprite.actions.find(event);

        if (it == nullptr)
            return;

        if (it->second == engine::Events::MOVE_UP)
        {
            // round the position to avoid pixel blurring
            sprite.position.y = std::round(sprite.position.y - 1000 * clock.getElapsedTime());
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.y -= std::round(1000 * clock.getElapsedTime());
            }
        }
        if (it->second == engine::Events::MOVE_DOWN)
        {
            sprite.position.y += std::round(1000 * clock.getElapsedTime());
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.y += std::round(1000 * clock.getElapsedTime());
            }
        }
        if (it->second == engine::Events::MOVE_LEFT)
        {
            sprite.position.x -= std::round(1000 * clock.getElapsedTime());
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.x -= std::round(1000 * clock.getElapsedTime());
            }
        }
        if (it->second == engine::Events::MOVE_RIGHT)
        {
            sprite.position.x += std::round(1000 * clock.getElapsedTime());
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.x += std::round(1000 * clock.getElapsedTime());
            }
        }
    }
}