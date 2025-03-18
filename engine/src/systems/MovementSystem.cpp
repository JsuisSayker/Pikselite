#include "systems/MovementSystem.hpp"

void MovementSystem::update(float deltaTime, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events)
{
    if (events.empty())
        return;

    graphic::EventType event = events.back();

    for (auto &sprite : sprites)
    {

        if (sprite.actions[event] == engine::Events::MOVE_UP)
        {
            sprite.position.y -= 50 * deltaTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.y -= 50 * deltaTime;
            }
        }
        else if (sprite.actions[event] == engine::Events::MOVE_DOWN)
        {
            sprite.position.y += 50 * deltaTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.y += 50 * deltaTime;
            }
        }
        else if (sprite.actions[event] == engine::Events::MOVE_LEFT)
        {
            sprite.position.x -= 50 * deltaTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.x -= 50 * deltaTime;
            }
        }
        else if (sprite.actions[event] == engine::Events::MOVE_RIGHT)
        {
            sprite.position.x += 50 * deltaTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.x += 50 * deltaTime;
            }
        }
    }
}