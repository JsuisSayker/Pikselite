#include "systems/MovementSystem.hpp"

MovementSystem::MovementSystem()
{
}

MovementSystem::~MovementSystem()
{
}

void MovementSystem::update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events)
{
    if (events.empty())
    {
        if (_timer.getElapsedTime() > 0.2)
        {
            _timer.restart();
            _event = graphic::EventType::NONE;
            return;
        }
    }
    else
    {
        graphic::EventType event = events.back();

        if (event != _event || _timer.getElapsedTime() > 0.2)
        {
            _event = event;
            _timer.restart();
        }
    }

    double elapsedTime = clock.getElapsedTime();

    for (graphic::Sprite &sprite : sprites)
    {
        auto it = sprite.actions.find(_event);

        if (it == nullptr)
            continue;

        if (it->second == engine::Events::MOVE_UP)
        {
            sprite.position.y -= 100 * elapsedTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.y -= 100 * elapsedTime;
            }
        }
        if (it->second == engine::Events::MOVE_DOWN)
        {
            sprite.position.y += 100 * elapsedTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.y += 100 * elapsedTime;
            }
        }
        if (it->second == engine::Events::MOVE_LEFT)
        {
            sprite.position.x -= 100 * elapsedTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.x -= 100 * elapsedTime;
            }
        }
        if (it->second == engine::Events::MOVE_RIGHT)
        {
            sprite.position.x += 100 * elapsedTime;
            for (auto &pixel : sprite.pixels)
            {
                pixel.position.x += 100 * elapsedTime;
            }
        }
    }
}