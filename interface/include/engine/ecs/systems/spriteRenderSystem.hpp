#pragma once

#include "engine/ecs/ISystem.hpp"
#include "engine/ecs/components/spriteComponent.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/managers/componentManager.hpp"

#include <graphics/graphicsEnum.hpp>
#include <graphics/renderer/camera.hpp>
#include <graphics/renderer/renderer.hpp>
#include <iostream>

namespace ecs::systems
{
    class SpriteRenderSystem : public ISystem
    {
      public:
        // The renderer and camera are injected from the outside
        SpriteRenderSystem(graphics::Renderer* renderer, graphics::Camera2D* camera)
            : _renderer(renderer), _camera(camera)
        {
        }

        void init() override {}
        bool shouldRunInUpdate() const override
        {
            return false;
        }

        /**
         * @brief Updates the sprite render system.
         * @param dt The delta time since the last update.
         * @param componentManager The component manager.
         */
        void update(double dt, engine::ComponentManager& componentManager) override;

      private:
        // The renderer is needed to draw sprites on the screen
        graphics::Renderer* _renderer = nullptr;

        // The camera is needed to convert world coordinates to screen coordinates for rendering
        graphics::Camera2D* _camera = nullptr;
    };

} // namespace ecs::systems
