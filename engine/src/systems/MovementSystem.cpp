#include "systems/MovementSystem.hpp"

void MovementSystem::update(float deltaTime, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events)
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
            sprite.position.y -= 100 * deltaTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.y -= 100 * deltaTime;
            }
        }
        if (it->second == engine::Events::MOVE_DOWN)
        {
            sprite.position.y += 100 * deltaTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.y += 100 * deltaTime;
            }
        }
        if (it->second == engine::Events::MOVE_LEFT)
        {
            sprite.position.x -= 100 * deltaTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.x -= 100 * deltaTime;
            }
        }
        if (it->second == engine::Events::MOVE_RIGHT)
        {
            sprite.position.x += 100 * deltaTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.x += 100 * deltaTime;
            }
        }
    }
}