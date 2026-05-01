#pragma once

#include "engine/ecs/ISystem.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/velocityComponent.hpp"
#include "engine/managers/componentManager.hpp"
#include "engine/managers/luaManager.hpp"

#include <SDL2/SDL.h>
#include <memory>
#include <string>

namespace ecs::systems
{
    /// ECS System that runs a Lua script each frame.
    /// The script can read keyboard state and modify entity Transform/Velocity.
    class ScriptSystem : public ISystem
    {
      public:
        ScriptSystem() = default;

        /**
         * @brief Initializes the ScriptSystem.
         *
         */
        void init() override
        {
            _lua = std::make_unique<engine::LuaManager>();
            registerBindings();
        }

        /**
         * @brief Loads a Lua script file.
         * @param filepath The path to the Lua script file.
         * @return True if the script was loaded successfully, false otherwise.
         */
        bool loadScript(const std::string& filepath)
        {
            return _lua && _lua->loadScript(filepath);
        }

        /**
         * @brief Updates the script system.
         * This function is called every frame and allows the Lua script to modify entity components
         * based on input or other logic defined in the script.
         * @param dt The delta time since the last update.
         * @param componentManager The component manager.
         */
        void update(double dt, engine::ComponentManager& componentManager) override
        {
            if (!_lua || !_lua->hasFunction("update"))
                return;

            // Snapshot the keyboard state and expose it to Lua each frame
            updateKeyboardState();

            // For each entity that has Transform + Velocity, push it into Lua
            for (auto entity : entities)
            {
                auto& transform = componentManager.getComponent<components::Transform>(entity);
                auto& velocity  = componentManager.getComponent<components::Velocity>(entity);

                // Create a table representing the entity for Lua
                sol::state& lua = _lua->state();
                lua["entity"]   = lua.create_table_with(
                    "id", entity, "x", transform.x, "y", transform.y, "vx", velocity.vx, "vy",
                    velocity.vy, "rotation", transform.rotation, "scaleX", transform.scaleX,
                    "scaleY", transform.scaleY);

                // Call update(dt) — the script modifies entity table
                _lua->call("update", dt);

                // Read back modified values
                sol::table ent = lua["entity"];
                if (ent.valid())
                {
                    transform.x        = ent.get_or<float>("x", transform.x);
                    transform.y        = ent.get_or<float>("y", transform.y);
                    velocity.vx        = ent.get_or<float>("vx", velocity.vx);
                    velocity.vy        = ent.get_or<float>("vy", velocity.vy);
                    transform.rotation = ent.get_or<float>("rotation", transform.rotation);
                    transform.scaleX   = ent.get_or<float>("scaleX", transform.scaleX);
                    transform.scaleY   = ent.get_or<float>("scaleY", transform.scaleY);
                }
            }
        }

      private:
        // Lua manager instance
        std::unique_ptr<engine::LuaManager> _lua;

        /**
         * @brief Registers C++ functions and variables to be accessible from Lua scripts.
         * This includes input handling functions and utility functions like logging.
         */
        void registerBindings()
        {
            sol::state& lua = _lua->state();

            // ── Input ────────────────────────────────────────────
            // is_key_pressed("left") → true/false
            lua.set_function("is_key_pressed",
                             [](const std::string& key) -> bool
                             {
                                 const Uint8* state = SDL_GetKeyboardState(nullptr);
                                 if (key == "up" || key == "w")
                                     return state[SDL_SCANCODE_W];
                                 if (key == "down" || key == "s")
                                     return state[SDL_SCANCODE_S];
                                 if (key == "left" || key == "a")
                                     return state[SDL_SCANCODE_A];
                                 if (key == "right" || key == "d")
                                     return state[SDL_SCANCODE_D];
                                 if (key == "space")
                                     return state[SDL_SCANCODE_SPACE];
                                 if (key == "lshift")
                                     return state[SDL_SCANCODE_LSHIFT];
                                 if (key == "escape")
                                     return state[SDL_SCANCODE_ESCAPE];
                                 return false;
                             });

            // ── Utility ──────────────────────────────────────────
            lua.set_function("log", [](const std::string& msg)
                             { std::cout << "[Lua] " << msg << std::endl; });
        }

        void updateKeyboardState()
        {
            // SDL_PumpEvents is already called by SDL_PollEvent in Core,
            // so SDL_GetKeyboardState is always up to date here.
        }
    };
} // namespace ecs::systems
