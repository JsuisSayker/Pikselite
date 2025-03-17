#include "systems/MovementSystem.hpp"

void MovementSystem::update(float deltaTime, std::vector<graphic::Sprite> sprites)
{
    for (auto &sprite : sprites)
    {
        // if (sprite.actions.find(engine::Events::MOVE_UP) != sprite.actions.end())
        // {
        //     sprite.position.y -= 1;
        // }

        // if (sprite.actions.find(engine::Events::MOVE_DOWN) != sprite.actions.end())
        // {
        //     sprite.position.y += 1;
        // }

        // if (sprite.actions.find(engine::Events::MOVE_LEFT) != sprite.actions.end())
        // {
        //     sprite.position.x -= 1;
        // }

        // if (sprite.actions.find(engine::Events::MOVE_RIGHT) != sprite.actions.end())
        // {
        //     sprite.position.x += 1;
        // }
    }
}