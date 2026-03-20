#pragma once

#include <iostream>

#include "engine/ecs/ISystem.hpp"
#include "engine/managers/componentManager.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/spriteComponent.hpp"

#include <graphics/graphicsEnum.hpp>
#include <graphics/renderer/renderer.hpp>
#include <graphics/renderer/camera.hpp>

namespace ecs::systems
{
    class SpriteRenderSystem : public ISystem
    {
    public:
        // The renderer and camera are injected from the outside
        SpriteRenderSystem(graphics::Renderer *renderer, graphics::Camera2D *camera)
            : _renderer(renderer), _camera(camera) {}

        void init() override {}

        void update(double dt, engine::ComponentManager &componentManager) override;

    private:
        graphics::Renderer  *_renderer = nullptr;
        graphics::Camera2D  *_camera   = nullptr;
    };

} // namespace ecs::systems
