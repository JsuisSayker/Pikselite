#pragma once

#include "engine/ecs/ISystem.hpp"
#include "engine/ecs/components/physicsComponent.hpp"
#include "engine/ecs/components/scriptComponent.hpp"
#include "engine/ecs/components/spriteComponent.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/velocityComponent.hpp"
#include "engine/events/eventBus.hpp"
#include "engine/events/events.hpp"
#include "engine/managers/componentManager.hpp"
#include "engine/managers/entityManager.hpp"
#include "engine/managers/luaManager.hpp"
#include "engine/managers/systemManager.hpp"
#include "engine/pixels/simulation/chunk.hpp"
#include "engine/pixels/simulation/element.hpp"

#include <SDL2/SDL.h>
#include <cctype>
#include <cmath>
#include <graphics/renderer/camera.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ecs::systems
{
    class ScriptSystem : public ISystem
    {
      public:
        ScriptSystem(engine::EntityManager*    entityManager = nullptr,
                     engine::SystemManager*    systemManager = nullptr,
                     engine::events::EventBus* eventBus = nullptr, ChunkGrid* chunkGrid = nullptr,
                     graphics::Camera2D* camera = nullptr)
            : _entityManager(entityManager), _systemManager(systemManager), _eventBus(eventBus),
              _chunkGrid(chunkGrid), _camera(camera)
        {
        }

        /**
         * @brief Initializes the ScriptSystem.
         *
         */
        void init() override
        {
            if (_eventBus)
            {
                _collisionEnterHandlerId =
                    _eventBus->subscribe<engine::events::CollisionEnterEvent>(
                        [this](const engine::events::CollisionEnterEvent& ev)
                        {
                            if (ev.entityA == 0 || ev.entityB == 0 || ev.entityA == ev.entityB)
                                return;
                            _pendingCollisionEnter[ev.entityA].push_back(ev.entityB);
                            _pendingCollisionEnter[ev.entityB].push_back(ev.entityA);
                        });

                _collisionExitHandlerId = _eventBus->subscribe<engine::events::CollisionExitEvent>(
                    [this](const engine::events::CollisionExitEvent& ev)
                    {
                        if (ev.entityA == 0 || ev.entityB == 0 || ev.entityA == ev.entityB)
                            return;
                        _pendingCollisionExit[ev.entityA].push_back(ev.entityB);
                        _pendingCollisionExit[ev.entityB].push_back(ev.entityA);
                    });
            }
        }

        bool loadScript(const std::string& /*filepath*/)
        {
            return true;
        }

        void setEntityName(ecs::EntityID entity, const std::string& name)
        {
            _entityNames[entity] = name;
        }

        void entityDestroyed(ecs::EntityID entity) override
        {
            _scriptInstances.erase(entity);
            _pendingCollisionEnter.erase(entity);
            _pendingCollisionExit.erase(entity);
            _entityNames.erase(entity);
        }

        void shutdown() override
        {
            _scriptInstances.clear();
            _pendingCollisionEnter.clear();
            _pendingCollisionExit.clear();
            _entityNames.clear();
            _pixelCommands.clear();
            _victory = false;
            _lose    = false;
            _victoryReason.clear();
            _loseReason.clear();
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
            if (!_entityManager)
                return;

            updateKeyboardState();
            _activeComponentManager = &componentManager;
            pruneStaleScriptInstances();

            for (auto entity : entities)
            {
                if (!componentManager.hasComponent<components::Script>(entity))
                    continue;

                auto& script = componentManager.getComponent<components::Script>(entity);
                if (!script.enabled || script.scriptPath.empty())
                    continue;

                ScriptInstance& instance = _scriptInstances[entity];
                if (!ensureScriptLoaded(entity, script.scriptPath, instance))
                    continue;

                sol::state& lua = instance.lua->state();
                lua["entity"]   = buildEntityTable(lua, entity);

                dispatchCollisionCallbacks(entity, instance);
                dispatchGameEndCallbacks(instance);

                if (instance.lua->hasFunction("update"))
                {
                    instance.lua->call("update", dt);
                }

                applyEntityTableChanges(entity, lua["entity"]);

                if (_camera && _cameraFollowEntity == entity &&
                    _activeComponentManager->hasComponent<components::Transform>(entity))
                {
                    const auto& transform =
                        _activeComponentManager->getComponent<components::Transform>(entity);
                    _camera->setPosition(transform.x, transform.y);
                }
            }

            flushPixelCommands();
            _activeComponentManager = nullptr;
        }

      private:
        struct ScriptInstance
        {
            std::unique_ptr<engine::LuaManager> lua;
            std::string                         scriptPath;
            bool                                victoryHandled = false;
            bool                                loseHandled    = false;
        };

        struct PixelCommand
        {
            int                  x    = 0;
            int                  y    = 0;
            Element::ElementType type = Element::EMPTY;
        };

        engine::EntityManager*    _entityManager      = nullptr;
        engine::SystemManager*    _systemManager      = nullptr;
        engine::events::EventBus* _eventBus           = nullptr;
        ChunkGrid*                _chunkGrid          = nullptr;
        graphics::Camera2D*       _camera             = nullptr;
        ecs::EntityID             _cameraFollowEntity = 0;

        engine::ComponentManager*                         _activeComponentManager = nullptr;
        std::unordered_map<ecs::EntityID, ScriptInstance> _scriptInstances;
        std::unordered_map<ecs::EntityID, std::vector<ecs::EntityID>> _pendingCollisionEnter;
        std::unordered_map<ecs::EntityID, std::vector<ecs::EntityID>> _pendingCollisionExit;
        std::unordered_map<ecs::EntityID, std::string>                _entityNames;
        std::vector<PixelCommand>                                     _pixelCommands;

        engine::events::EventBus::HandlerId _collisionEnterHandlerId = 0;
        engine::events::EventBus::HandlerId _collisionExitHandlerId  = 0;

        bool        _victory = false;
        bool        _lose    = false;
        std::string _victoryReason;
        std::string _loseReason;

        static std::string toLower(std::string value)
        {
            for (char& c : value)
            {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            return value;
        }

        bool ensureScriptLoaded(ecs::EntityID entity, const std::string& scriptPath,
                                ScriptInstance& instance)
        {
            if (!instance.lua || instance.scriptPath != scriptPath)
            {
                instance.lua = std::make_unique<engine::LuaManager>();
                registerBindings(*instance.lua);
                instance.scriptPath     = scriptPath;
                instance.victoryHandled = false;
                instance.loseHandled    = false;

                if (!instance.lua->loadScript(scriptPath))
                {
                    std::cerr << "[ScriptSystem] Failed to load script for entity " << entity
                              << ": " << scriptPath << std::endl;
                    return false;
                }
            }

            return true;
        }

        void pruneStaleScriptInstances()
        {
            std::vector<ecs::EntityID> stale;
            stale.reserve(_scriptInstances.size());

            for (const auto& [entityId, _] : _scriptInstances)
            {
                if (!_entityManager->hasEntity(entityId))
                {
                    stale.push_back(entityId);
                }
            }

            for (ecs::EntityID entityId : stale)
            {
                entityDestroyed(entityId);
            }
        }

        void registerBindings(engine::LuaManager& luaManager)
        {
            sol::state& lua = luaManager.state();

            lua.set_function("is_key_pressed",
                             [](const std::string& key) -> bool
                             {
                                 const std::string lowerKey = toLower(key);
                                 const Uint8*      state    = SDL_GetKeyboardState(nullptr);
                                 if (lowerKey == "up" || lowerKey == "w")
                                     return state[SDL_SCANCODE_W];
                                 if (lowerKey == "down" || lowerKey == "s")
                                     return state[SDL_SCANCODE_S];
                                 if (lowerKey == "left" || lowerKey == "a")
                                     return state[SDL_SCANCODE_A];
                                 if (lowerKey == "right" || lowerKey == "d")
                                     return state[SDL_SCANCODE_D];
                                 if (lowerKey == "space")
                                     return state[SDL_SCANCODE_SPACE];
                                 if (lowerKey == "lshift")
                                     return state[SDL_SCANCODE_LSHIFT];
                                 if (lowerKey == "escape")
                                     return state[SDL_SCANCODE_ESCAPE];
                                 if (lowerKey == "e")
                                     return state[SDL_SCANCODE_E];
                                 if (lowerKey == "q")
                                     return state[SDL_SCANCODE_Q];
                                 if (lowerKey == "r")
                                     return state[SDL_SCANCODE_R];
                                 if (lowerKey == "f")
                                     return state[SDL_SCANCODE_F];
                                 return false;
                             });

            lua.set_function("log", [](const std::string& msg)
                             { std::cout << "[Lua] " << msg << std::endl; });

            lua.set_function("is_mouse_pressed",
                             [](const std::string& button) -> bool
                             {
                                 const std::string lowerButton = toLower(button);
                                 const Uint32      state = SDL_GetMouseState(nullptr, nullptr);
                                 if (lowerButton == "left")
                                     return (state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
                                 if (lowerButton == "right")
                                     return (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
                                 if (lowerButton == "middle")
                                     return (state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
                                 return false;
                             });

            lua.set_function("get_mouse_world_position",
                             [this](sol::this_state ts) -> sol::object
                             {
                                 sol::state_view view(ts);
                                 if (!_camera)
                                 {
                                     return sol::make_object(view, sol::nil);
                                 }

                                 SDL_Window* window = SDL_GetMouseFocus();
                                 if (!window)
                                 {
                                     return sol::make_object(view, sol::nil);
                                 }

                                 int width  = 0;
                                 int height = 0;
                                 SDL_GetWindowSize(window, &width, &height);

                                 int mouseX = 0;
                                 int mouseY = 0;
                                 SDL_GetMouseState(&mouseX, &mouseY);

                                 const glm::vec2 worldPos =
                                     _camera->screenToWorld(glm::vec2(static_cast<float>(mouseX),
                                                                      static_cast<float>(mouseY)),
                                                            width, height);
                                 sol::table out = view.create_table();
                                 out["x"]       = worldPos.x;
                                 out["y"]       = worldPos.y;
                                 return sol::make_object(view, out);
                             });

            lua.set_function("follow_entity",
                             [this](std::uint32_t entityId) { _cameraFollowEntity = entityId; });

            lua.set_function("clear_camera_follow", [this]() { _cameraFollowEntity = 0; });

            lua.set_function("get_entity",
                             [this](std::uint32_t id, sol::this_state ts) -> sol::object
                             {
                                 sol::state_view view(ts);
                                 if (!_entityManager || !_entityManager->hasEntity(id))
                                 {
                                     return sol::make_object(view, sol::nil);
                                 }

                                 return sol::make_object(view, buildEntityTable(view, id));
                             });

            lua.set_function("get_entity_by_name",
                             [this](const std::string& name, sol::this_state ts) -> sol::object
                             {
                                 sol::state_view view(ts);
                                 if (!_entityManager || !_activeComponentManager)
                                 {
                                     return sol::make_object(view, sol::nil);
                                 }

                                 for (const auto& entityPtr : _entityManager->getEntities())
                                 {
                                     if (!entityPtr)
                                         continue;
                                     const ecs::EntityID entityId = entityPtr->id;
                                     auto                it       = _entityNames.find(entityId);
                                     if (it != _entityNames.end() && it->second == name)
                                     {
                                         return sol::make_object(view,
                                                                 buildEntityTable(view, entityId));
                                     }
                                 }

                                 return sol::make_object(view, sol::nil);
                             });

            lua.set_function("get_entities",
                             [this](sol::this_state ts) -> sol::table
                             {
                                 sol::state_view view(ts);
                                 sol::table      out = view.create_table();
                                 if (!_entityManager)
                                     return out;

                                 int i = 1;
                                 for (const auto& entityPtr : _entityManager->getEntities())
                                 {
                                     if (!entityPtr)
                                         continue;
                                     out[i++] = buildEntityTable(view, entityPtr->id);
                                 }
                                 return out;
                             });

            lua.set_function(
                "create_entity",
                sol::overload([this]() -> std::uint32_t
                              { return createEntityFromTable(sol::optional<sol::table>()); },
                              [this](sol::table data) -> std::uint32_t
                              { return createEntityFromTable(data); }));

            lua.set_function("delete_entity",
                             [this](std::uint32_t id) -> bool { return deleteEntity(id); });

            lua.set_function("has_component",
                             [this](std::uint32_t id, const std::string& type) -> bool
                             { return hasComponent(id, type); });

            lua.set_function(
                "get_component",
                [this](std::uint32_t id, const std::string& type, sol::this_state ts) -> sol::object
                {
                    sol::state_view view(ts);
                    return getComponentAsObject(view, id, type);
                });

            lua.set_function(
                "set_component",
                [this](std::uint32_t id, const std::string& type, sol::table data) -> bool
                { return setComponentFromTable(id, type, data); });

            lua.set_function(
                "add_component",
                [this](std::uint32_t id, const std::string& type, sol::table data) -> bool
                { return addComponentFromTable(id, type, data); });

            lua.set_function("remove_component",
                             [this](std::uint32_t id, const std::string& type) -> bool
                             { return removeComponent(id, type); });

            lua.set_function("set_victory",
                             [this](const std::string& reason)
                             {
                                 if (_victory)
                                     return;
                                 _victory       = true;
                                 _victoryReason = reason;

                                 if (_eventBus)
                                 {
                                     auto ev    = std::make_unique<engine::events::VictoryEvent>();
                                     ev->reason = reason;
                                     _eventBus->publish(std::move(ev));
                                 }
                             });

            lua.set_function("set_lose",
                             [this](const std::string& reason)
                             {
                                 if (_lose)
                                     return;
                                 _lose       = true;
                                 _loseReason = reason;

                                 if (_eventBus)
                                 {
                                     auto ev    = std::make_unique<engine::events::LoseEvent>();
                                     ev->reason = reason;
                                     _eventBus->publish(std::move(ev));
                                 }
                             });

            lua.set_function("is_victory", [this]() -> bool { return _victory; });
            lua.set_function("is_lose", [this]() -> bool { return _lose; });

            lua.set_function("create_pixel", [this](int x, int y, sol::object element) -> bool
                             { return queuePixelWrite(x, y, element); });

            lua.set_function("create_pixels",
                             [this](sol::table entries) -> int
                             {
                                 int accepted = 0;
                                 for (const auto& kv : entries)
                                 {
                                     if (!kv.second.is<sol::table>())
                                         continue;

                                     sol::table  item    = kv.second.as<sol::table>();
                                     const int   x       = item.get_or("x", 0);
                                     const int   y       = item.get_or("y", 0);
                                     sol::object typeObj = item["type"];
                                     if (queuePixelWrite(x, y, typeObj))
                                     {
                                         ++accepted;
                                     }
                                 }
                                 return accepted;
                             });

            lua.set_function(
                "basic_chase",
                [this](std::uint32_t selfId, std::uint32_t targetId, float speed,
                       sol::this_state ts) -> bool
                {
                    if (!_activeComponentManager)
                        return false;
                    if (!_entityManager || !_entityManager->hasEntity(selfId) ||
                        !_entityManager->hasEntity(targetId))
                        return false;

                    if (!_activeComponentManager->hasComponent<components::Transform>(selfId) ||
                        !_activeComponentManager->hasComponent<components::Transform>(targetId) ||
                        !_activeComponentManager->hasComponent<components::Velocity>(selfId))
                    {
                        return false;
                    }

                    auto& selfTransform =
                        _activeComponentManager->getComponent<components::Transform>(selfId);
                    const auto& targetTransform =
                        _activeComponentManager->getComponent<components::Transform>(targetId);
                    auto& selfVelocity =
                        _activeComponentManager->getComponent<components::Velocity>(selfId);

                    const float dx     = targetTransform.x - selfTransform.x;
                    const float dy     = targetTransform.y - selfTransform.y;
                    const float distSq = dx * dx + dy * dy;
                    if (distSq < 0.0001f)
                    {
                        selfVelocity.vx = 0.0f;
                        selfVelocity.vy = 0.0f;
                        return true;
                    }

                    const float invDist = 1.0f / std::sqrt(distSq);
                    selfVelocity.vx     = dx * invDist * speed;
                    selfVelocity.vy     = dy * invDist * speed;

                    // Keep the per-frame Lua entity snapshot in sync so applyEntityTableChanges
                    // doesn't overwrite the chase velocity right after this call.
                    sol::state_view view(ts);
                    sol::object     entityObj = view["entity"];
                    if (entityObj.valid() && entityObj.is<sol::table>())
                    {
                        sol::table entityTable = entityObj.as<sol::table>();
                        if (entityTable.get_or("id", static_cast<std::uint32_t>(0)) == selfId)
                        {
                            entityTable["vx"] = selfVelocity.vx;
                            entityTable["vy"] = selfVelocity.vy;
                        }
                    }

                    return true;
                });
        }

        Element::ElementType parseElementType(sol::object element) const
        {
            if (element.is<int>())
            {
                int v = element.as<int>();
                if (v >= 0 && v <= 65535)
                    return static_cast<Element::ElementType>(v);
                return Element::EMPTY;
            }

            if (element.is<double>())
            {
                int v = static_cast<int>(element.as<double>());
                if (v >= 0 && v <= 65535)
                    return static_cast<Element::ElementType>(v);
                return Element::EMPTY;
            }

            if (element.is<std::string>())
            {
                const std::string target = toLower(element.as<std::string>());
                for (int i = 0; i < 256; ++i)
                {
                    if (toLower(g_elements[i].name) == target)
                    {
                        return static_cast<Element::ElementType>(i);
                    }
                }
            }

            return Element::EMPTY;
        }

        bool queuePixelWrite(int x, int y, sol::object element)
        {
            if (_chunkGrid == nullptr)
                return false;

            Element::ElementType type = parseElementType(element);
            _pixelCommands.push_back(PixelCommand{x, y, type});
            return true;
        }

        void flushPixelCommands()
        {
            if (_chunkGrid == nullptr)
            {
                _pixelCommands.clear();
                return;
            }

            for (const PixelCommand& command : _pixelCommands)
            {
                _chunkGrid->setPixel(command.x, command.y, {command.type, false});
            }

            _pixelCommands.clear();
        }

        std::uint32_t createEntityFromTable(const sol::optional<sol::table>& data)
        {
            if (!_entityManager || !_activeComponentManager)
                return 0;

            const ecs::Entity   entity   = _entityManager->createEntity();
            const ecs::EntityID entityId = entity.id;

            if (data)
            {
                const sol::table& table        = *data;
                const sol::object nameObj      = table["name"];
                const sol::object transformObj = table["transform"];
                const sol::object velocityObj  = table["velocity"];
                const sol::object spriteObj    = table["sprite"];
                const sol::object physicsObj   = table["physics"];
                const sol::object scriptObj    = table["script"];

                if (nameObj.valid() && nameObj.is<std::string>())
                {
                    _entityNames[entityId] = nameObj.as<std::string>();
                }

                if (transformObj.valid() && transformObj.is<sol::table>())
                {
                    const sol::table      t = transformObj.as<sol::table>();
                    components::Transform transform{};
                    transform.enabled  = t.get_or("enabled", true);
                    transform.x        = t.get_or("x", 0.0f);
                    transform.y        = t.get_or("y", 0.0f);
                    transform.rotation = t.get_or("rotation", 0.0f);
                    transform.scaleX   = t.get_or("scaleX", 1.0f);
                    transform.scaleY   = t.get_or("scaleY", 1.0f);
                    transform.prevX    = t.get_or("prevX", transform.x);
                    transform.prevY    = t.get_or("prevY", transform.y);
                    _activeComponentManager->addComponent(entityId, transform);
                }

                if (velocityObj.valid() && velocityObj.is<sol::table>())
                {
                    const sol::table     v = velocityObj.as<sol::table>();
                    components::Velocity velocity{};
                    velocity.enabled = v.get_or("enabled", true);
                    velocity.vx      = v.get_or("vx", 0.0f);
                    velocity.vy      = v.get_or("vy", 0.0f);
                    _activeComponentManager->addComponent(entityId, velocity);
                }

                if (spriteObj.valid() && spriteObj.is<sol::table>())
                {
                    const sol::table   s = spriteObj.as<sol::table>();
                    components::Sprite sprite{};
                    sprite.enabled     = s.get_or("enabled", true);
                    sprite.texturePath = s.get_or("texturePath", sprite.texturePath);
                    sprite.width       = s.get_or("width", sprite.width);
                    sprite.height      = s.get_or("height", sprite.height);
                    _activeComponentManager->addComponent(entityId, sprite);
                }

                if (physicsObj.valid() && physicsObj.is<sol::table>())
                {
                    const sol::table        p = physicsObj.as<sol::table>();
                    components::PhysicsBody physics{};
                    physics.enabled = p.get_or("enabled", true);
                    const std::string bodyType =
                        toLower(p.get_or("bodyType", std::string("dynamic")));
                    if (bodyType == "static")
                        physics.bodyType = b2_staticBody;
                    else if (bodyType == "kinematic")
                        physics.bodyType = b2_kinematicBody;
                    else
                        physics.bodyType = b2_dynamicBody;
                    physics.fixedRotation = p.get_or("fixedRotation", false);
                    physics.density       = p.get_or("density", 1.0f);
                    physics.friction      = p.get_or("friction", 0.4f);
                    physics.restitution   = p.get_or("restitution", 0.1f);
                    _activeComponentManager->addComponent(entityId, physics);
                }

                if (scriptObj.valid())
                {
                    components::Script script{};
                    if (scriptObj.is<std::string>())
                    {
                        script.scriptPath = scriptObj.as<std::string>();
                    }
                    else if (scriptObj.is<sol::table>())
                    {
                        const sol::table st = scriptObj.as<sol::table>();
                        script.enabled      = st.get_or("enabled", true);
                        script.scriptPath   = st.get_or("scriptPath", script.scriptPath);
                    }
                    _activeComponentManager->addComponent(entityId, script);
                }
            }

            return entityId;
        }

        bool deleteEntity(ecs::EntityID entityId)
        {
            if (!_entityManager || !_entityManager->hasEntity(entityId))
                return false;

            if (_activeComponentManager)
            {
                _activeComponentManager->entityDestroyed(entityId);
            }

            if (_systemManager)
            {
                _systemManager->entityDestroyed(entityId);
            }

            _entityManager->destroyEntity(ecs::Entity(entityId));
            entityDestroyed(entityId);
            return true;
        }

        bool hasComponent(ecs::EntityID entityId, const std::string& type) const
        {
            if (!_entityManager || !_activeComponentManager || !_entityManager->hasEntity(entityId))
                return false;

            const std::string key = toLower(type);
            if (key == "transform")
                return _activeComponentManager->hasComponent<components::Transform>(entityId);
            if (key == "velocity")
                return _activeComponentManager->hasComponent<components::Velocity>(entityId);
            if (key == "sprite")
                return _activeComponentManager->hasComponent<components::Sprite>(entityId);
            if (key == "physics" || key == "physicsbody")
                return _activeComponentManager->hasComponent<components::PhysicsBody>(entityId);
            if (key == "script")
                return _activeComponentManager->hasComponent<components::Script>(entityId);
            if (key == "name")
                return _entityNames.find(entityId) != _entityNames.end();
            return false;
        }

        sol::object getComponentAsObject(sol::state_view lua, ecs::EntityID entityId,
                                         const std::string& type)
        {
            if (!hasComponent(entityId, type))
            {
                return sol::make_object(lua, sol::nil);
            }

            const std::string key   = toLower(type);
            sol::table        table = lua.create_table();

            if (key == "transform")
            {
                const auto& t =
                    _activeComponentManager->getComponent<components::Transform>(entityId);
                table["enabled"]  = t.enabled;
                table["x"]        = t.x;
                table["y"]        = t.y;
                table["rotation"] = t.rotation;
                table["scaleX"]   = t.scaleX;
                table["scaleY"]   = t.scaleY;
                table["prevX"]    = t.prevX;
                table["prevY"]    = t.prevY;
            }
            else if (key == "velocity")
            {
                const auto& v =
                    _activeComponentManager->getComponent<components::Velocity>(entityId);
                table["enabled"] = v.enabled;
                table["vx"]      = v.vx;
                table["vy"]      = v.vy;
            }
            else if (key == "sprite")
            {
                const auto& s = _activeComponentManager->getComponent<components::Sprite>(entityId);
                table["enabled"]     = s.enabled;
                table["texturePath"] = s.texturePath;
                table["width"]       = s.width;
                table["height"]      = s.height;
                table["loaded"]      = s.loaded;
            }
            else if (key == "physics" || key == "physicsbody")
            {
                const auto& p =
                    _activeComponentManager->getComponent<components::PhysicsBody>(entityId);
                table["enabled"] = p.enabled;
                table["bodyType"] =
                    p.bodyType == b2_staticBody
                        ? "static"
                        : (p.bodyType == b2_kinematicBody ? "kinematic" : "dynamic");
                table["fixedRotation"] = p.fixedRotation;
                table["density"]       = p.density;
                table["friction"]      = p.friction;
                table["restitution"]   = p.restitution;
            }
            else if (key == "script")
            {
                const auto& s = _activeComponentManager->getComponent<components::Script>(entityId);
                table["enabled"]    = s.enabled;
                table["scriptPath"] = s.scriptPath;
            }
            else if (key == "name")
            {
                auto it        = _entityNames.find(entityId);
                table["value"] = (it != _entityNames.end()) ? it->second : std::string();
            }

            return sol::make_object(lua, table);
        }

        bool setComponentFromTable(ecs::EntityID entityId, const std::string& type,
                                   const sol::table& data)
        {
            if (!hasComponent(entityId, type))
                return false;

            const std::string key = toLower(type);
            if (key == "transform")
            {
                auto& t    = _activeComponentManager->getComponent<components::Transform>(entityId);
                t.enabled  = data.get_or("enabled", t.enabled);
                t.x        = data.get_or("x", t.x);
                t.y        = data.get_or("y", t.y);
                t.rotation = data.get_or("rotation", t.rotation);
                t.scaleX   = data.get_or("scaleX", t.scaleX);
                t.scaleY   = data.get_or("scaleY", t.scaleY);
                t.prevX    = data.get_or("prevX", t.prevX);
                t.prevY    = data.get_or("prevY", t.prevY);
                return true;
            }
            if (key == "velocity")
            {
                auto& v   = _activeComponentManager->getComponent<components::Velocity>(entityId);
                v.enabled = data.get_or("enabled", v.enabled);
                v.vx      = data.get_or("vx", v.vx);
                v.vy      = data.get_or("vy", v.vy);
                return true;
            }
            if (key == "sprite")
            {
                auto& s       = _activeComponentManager->getComponent<components::Sprite>(entityId);
                s.enabled     = data.get_or("enabled", s.enabled);
                s.texturePath = data.get_or("texturePath", s.texturePath);
                s.width       = data.get_or("width", s.width);
                s.height      = data.get_or("height", s.height);
                return true;
            }
            if (key == "physics" || key == "physicsbody")
            {
                auto& p = _activeComponentManager->getComponent<components::PhysicsBody>(entityId);
                p.enabled = data.get_or("enabled", p.enabled);
                const std::string bodyType =
                    toLower(data.get_or("bodyType", std::string("dynamic")));
                if (bodyType == "static")
                    p.bodyType = b2_staticBody;
                else if (bodyType == "kinematic")
                    p.bodyType = b2_kinematicBody;
                else
                    p.bodyType = b2_dynamicBody;
                p.fixedRotation = data.get_or("fixedRotation", p.fixedRotation);
                p.density       = data.get_or("density", p.density);
                p.friction      = data.get_or("friction", p.friction);
                p.restitution   = data.get_or("restitution", p.restitution);
                return true;
            }
            if (key == "script")
            {
                auto& s      = _activeComponentManager->getComponent<components::Script>(entityId);
                s.enabled    = data.get_or("enabled", s.enabled);
                s.scriptPath = data.get_or("scriptPath", s.scriptPath);
                return true;
            }
            if (key == "name")
            {
                auto              it      = _entityNames.find(entityId);
                const std::string current = (it != _entityNames.end()) ? it->second : std::string();
                _entityNames[entityId]    = data.get_or("value", current);
                return true;
            }

            return false;
        }

        bool addComponentFromTable(ecs::EntityID entityId, const std::string& type,
                                   const sol::table& data)
        {
            if (!_entityManager || !_activeComponentManager || !_entityManager->hasEntity(entityId))
                return false;

            if (hasComponent(entityId, type))
                return setComponentFromTable(entityId, type, data);

            const std::string key = toLower(type);
            if (key == "transform")
            {
                components::Transform t{};
                t.enabled  = data.get_or("enabled", true);
                t.x        = data.get_or("x", 0.0f);
                t.y        = data.get_or("y", 0.0f);
                t.rotation = data.get_or("rotation", 0.0f);
                t.scaleX   = data.get_or("scaleX", 1.0f);
                t.scaleY   = data.get_or("scaleY", 1.0f);
                t.prevX    = data.get_or("prevX", t.x);
                t.prevY    = data.get_or("prevY", t.y);
                _activeComponentManager->addComponent(entityId, t);
                return true;
            }
            if (key == "velocity")
            {
                components::Velocity v{};
                v.enabled = data.get_or("enabled", true);
                v.vx      = data.get_or("vx", 0.0f);
                v.vy      = data.get_or("vy", 0.0f);
                _activeComponentManager->addComponent(entityId, v);
                return true;
            }
            if (key == "sprite")
            {
                components::Sprite s{};
                s.enabled     = data.get_or("enabled", true);
                s.texturePath = data.get_or("texturePath", s.texturePath);
                s.width       = data.get_or("width", s.width);
                s.height      = data.get_or("height", s.height);
                _activeComponentManager->addComponent(entityId, s);
                return true;
            }
            if (key == "physics" || key == "physicsbody")
            {
                components::PhysicsBody p{};
                p.enabled = data.get_or("enabled", true);
                const std::string bodyType =
                    toLower(data.get_or("bodyType", std::string("dynamic")));
                if (bodyType == "static")
                    p.bodyType = b2_staticBody;
                else if (bodyType == "kinematic")
                    p.bodyType = b2_kinematicBody;
                else
                    p.bodyType = b2_dynamicBody;
                p.fixedRotation = data.get_or("fixedRotation", false);
                p.density       = data.get_or("density", 1.0f);
                p.friction      = data.get_or("friction", 0.4f);
                p.restitution   = data.get_or("restitution", 0.1f);
                _activeComponentManager->addComponent(entityId, p);
                return true;
            }
            if (key == "script")
            {
                components::Script s{};
                s.enabled    = data.get_or("enabled", true);
                s.scriptPath = data.get_or("scriptPath", s.scriptPath);
                _activeComponentManager->addComponent(entityId, s);
                return true;
            }
            if (key == "name")
            {
                _entityNames[entityId] = data.get_or("value", std::string());
                return true;
            }

            return false;
        }

        bool removeComponent(ecs::EntityID entityId, const std::string& type)
        {
            if (!hasComponent(entityId, type))
                return false;

            const std::string key = toLower(type);
            if (key == "transform")
            {
                _activeComponentManager->removeComponent<components::Transform>(entityId);
                return true;
            }
            if (key == "velocity")
            {
                _activeComponentManager->removeComponent<components::Velocity>(entityId);
                return true;
            }
            if (key == "sprite")
            {
                _activeComponentManager->removeComponent<components::Sprite>(entityId);
                return true;
            }
            if (key == "physics" || key == "physicsbody")
            {
                _activeComponentManager->removeComponent<components::PhysicsBody>(entityId);
                return true;
            }
            if (key == "script")
            {
                _activeComponentManager->removeComponent<components::Script>(entityId);
                _scriptInstances.erase(entityId);
                return true;
            }
            if (key == "name")
            {
                _entityNames.erase(entityId);
                return true;
            }

            return false;
        }

        sol::table buildEntityTable(sol::state_view lua, ecs::EntityID entityId)
        {
            sol::table table = lua.create_table();
            table["id"]      = entityId;

            if (!_activeComponentManager || !_entityManager || !_entityManager->hasEntity(entityId))
                return table;

            auto itName = _entityNames.find(entityId);
            if (itName != _entityNames.end())
            {
                table["name"] = itName->second;
            }

            if (_activeComponentManager->hasComponent<components::Transform>(entityId))
            {
                const auto& transform =
                    _activeComponentManager->getComponent<components::Transform>(entityId);
                table["x"]        = transform.x;
                table["y"]        = transform.y;
                table["rotation"] = transform.rotation;
                table["scaleX"]   = transform.scaleX;
                table["scaleY"]   = transform.scaleY;
            }

            if (_activeComponentManager->hasComponent<components::Velocity>(entityId))
            {
                const auto& velocity =
                    _activeComponentManager->getComponent<components::Velocity>(entityId);
                table["vx"] = velocity.vx;
                table["vy"] = velocity.vy;
            }

            if (_activeComponentManager->hasComponent<components::Script>(entityId))
            {
                const auto& script =
                    _activeComponentManager->getComponent<components::Script>(entityId);
                table["scriptPath"] = script.scriptPath;
            }

            return table;
        }

        void applyEntityTableChanges(ecs::EntityID entityId, const sol::table& table)
        {
            if (!_activeComponentManager || !_entityManager || !_entityManager->hasEntity(entityId))
                return;

            auto itName = _entityNames.find(entityId);
            if (itName != _entityNames.end())
            {
                itName->second = table.get_or("name", itName->second);
            }

            if (_activeComponentManager->hasComponent<components::Transform>(entityId))
            {
                auto& transform =
                    _activeComponentManager->getComponent<components::Transform>(entityId);
                transform.x        = table.get_or("x", transform.x);
                transform.y        = table.get_or("y", transform.y);
                transform.rotation = table.get_or("rotation", transform.rotation);
                transform.scaleX   = table.get_or("scaleX", transform.scaleX);
                transform.scaleY   = table.get_or("scaleY", transform.scaleY);
            }

            if (_activeComponentManager->hasComponent<components::Velocity>(entityId))
            {
                auto& velocity =
                    _activeComponentManager->getComponent<components::Velocity>(entityId);
                velocity.vx = table.get_or("vx", velocity.vx);
                velocity.vy = table.get_or("vy", velocity.vy);
            }
        }

        void dispatchCollisionCallbacks(ecs::EntityID entityId, ScriptInstance& instance)
        {
            if (!instance.lua)
                return;

            auto enterIt = _pendingCollisionEnter.find(entityId);
            if (enterIt != _pendingCollisionEnter.end() &&
                instance.lua->hasFunction("on_collision_enter"))
            {
                for (ecs::EntityID otherId : enterIt->second)
                {
                    instance.lua->call("on_collision_enter", otherId);
                }
                enterIt->second.clear();
            }

            auto exitIt = _pendingCollisionExit.find(entityId);
            if (exitIt != _pendingCollisionExit.end() &&
                instance.lua->hasFunction("on_collision_exit"))
            {
                for (ecs::EntityID otherId : exitIt->second)
                {
                    instance.lua->call("on_collision_exit", otherId);
                }
                exitIt->second.clear();
            }
        }

        void dispatchGameEndCallbacks(ScriptInstance& instance)
        {
            if (!instance.lua)
                return;

            if (_victory && !instance.victoryHandled)
            {
                if (instance.lua->hasFunction("on_victory"))
                {
                    instance.lua->call("on_victory", _victoryReason);
                }
                instance.victoryHandled = true;
            }

            if (_lose && !instance.loseHandled)
            {
                if (instance.lua->hasFunction("on_lose"))
                {
                    instance.lua->call("on_lose", _loseReason);
                }
                instance.loseHandled = true;
            }
        }

        void updateKeyboardState()
        {
            // SDL_GetKeyboardState is already synchronized by SDL_PollEvent in Core.
        }
    };
} // namespace ecs::systems