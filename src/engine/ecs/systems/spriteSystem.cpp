#include "engine/ecs/systems/spriteRenderSystem.hpp"
#include <tracy/Tracy.hpp>

#ifndef TRACY_ENABLE
// output a warning if profiling is disabled
#pragma message(                                                                                   \
    "Tracy profiling is disabled. To enable, set PIKSELITE_ENABLE_PROFILING=ON in CMake and rebuild.")
#error "Not set"
#endif

namespace ecs::systems
{
    void SpriteRenderSystem::update(double dt, engine::ComponentManager& componentManager)
    {
        ZoneScopedN("ECS::SpriteRenderSystem");
        (void)dt; // not used for rendering

        for (auto entity : entities)
        {
            auto& transform = componentManager.getComponent<components::Transform>(entity);
            auto& sprite = componentManager.getComponent<components::Sprite>(entity);

            // Lazy-load the texture the first time we see this entity
            if (!sprite.loaded && !sprite.texturePath.empty())
            {
                sprite.textureID = _renderer->loadTexture(sprite.texturePath);
                sprite.loaded = true;
            }

            if (sprite.textureID == 0)
                continue;

            graphics::Sprite2D s2d;
            s2d.position = {transform.x, transform.y};
            s2d.size = {sprite.width * transform.scaleX, sprite.height * transform.scaleY};
            s2d.textureID = sprite.textureID;

            _renderer->drawSprite(s2d, *_camera);
        }
    }
} // namespace ecs::systems