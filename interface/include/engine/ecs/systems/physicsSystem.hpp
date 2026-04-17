#pragma once

#include <vector>
#include <algorithm>
#include <cmath>

#include <box2d/box2d.h>

#include "engine/ecs/ISystem.hpp"
#include "engine/managers/componentManager.hpp"
#include "engine/physics/boxWorld.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/velocityComponent.hpp"
#include "engine/ecs/components/physicsComponent.hpp"
#include "engine/ecs/components/spriteComponent.hpp"

namespace ecs::systems
{
    class PhysicsSystem : public ISystem
    {
    public:
        explicit PhysicsSystem(engine::physics::BoxWorld* boxWorld)
            : _boxWorld(boxWorld) {}

        std::vector<b2BodyId> getDebugBodies() const
        {
            std::vector<b2BodyId> bodies;
            bodies.reserve(_bodyByEntity.size());
            for (const auto& [entity, bodyId] : _bodyByEntity)
            {
                if (!B2_IS_NULL(bodyId))
                {
                    bodies.push_back(bodyId);
                }
            }
            return bodies;
        }

        void init() override {}

        void entityDestroyed(ecs::EntityID entity) override
        {
            if (!_boxWorld || !_boxWorld->isValid())
            {
                return;
            }

            auto physicsIt = _bodyByEntity.find(entity);
            if (physicsIt == _bodyByEntity.end())
            {
                return;
            }

            const b2BodyId bodyId = physicsIt->second;
            if (!B2_IS_NULL(bodyId))
            {
                b2DestroyBody(bodyId);
            }

            _bodyByEntity.erase(physicsIt);
        }

        void update(double dt, engine::ComponentManager& componentManager) override
        {
            if (!_boxWorld || !_boxWorld->isValid())
            {
                return;
            }

            const b2WorldId worldId = _boxWorld->getWorldId();

            for (auto entity : entities)
            {
                auto& transform = componentManager.getComponent<components::Transform>(entity);
                auto& physics = componentManager.getComponent<components::PhysicsBody>(entity);
                
                if (!physics.enabled)
                {
                    continue;
                }

                const bool hasVelocity = componentManager.hasComponent<components::Velocity>(entity);
                components::Velocity* velocity = hasVelocity
                    ? &componentManager.getComponent<components::Velocity>(entity)
                    : nullptr;

                const float desiredHorizontalVelocity = velocity ? velocity->vx : 0.0f;
                const float desiredVerticalVelocity = velocity ? velocity->vy : 0.0f;

                if (B2_IS_NULL(physics.bodyId))
                {
                    b2BodyDef bodyDef = b2DefaultBodyDef();
                    bodyDef.type = physics.bodyType;
                    bodyDef.position = {transform.x, transform.y};
                    bodyDef.rotation = b2MakeRot(transform.rotation);
                    bodyDef.fixedRotation = physics.fixedRotation;
                    physics.bodyId = b2CreateBody(worldId, &bodyDef);
                    _bodyByEntity[entity] = physics.bodyId;

                    b2ShapeDef shapeDef = b2DefaultShapeDef();
                    shapeDef.density = physics.density;
                    shapeDef.material.friction = physics.friction;
                    shapeDef.material.restitution = physics.restitution;

                    bool createdShape = false;

                    for (const auto& triangle : physics.triangles)
                    {
                        b2Vec2 points[3] = {
                            {triangle.a.x, triangle.a.y},
                            {triangle.b.x, triangle.b.y},
                            {triangle.c.x, triangle.c.y},
                        };

                        b2Hull hull = b2ComputeHull(points, 3);
                        if (hull.count == 3)
                        {
                            b2Polygon polygon = b2MakePolygon(&hull, 0.0f);
                            b2CreatePolygonShape(physics.bodyId, &shapeDef, &polygon);
                            createdShape = true;
                        }
                    }

                    // Fallback collider for sprite-based objects: create a box from sprite size
                    // when no custom triangles are provided.
                    if (!createdShape)
                    {
                        float width = PIXEL_SIZE;
                        float height = PIXEL_SIZE;

                        if (componentManager.hasComponent<components::Sprite>(entity))
                        {
                            const auto& sprite = componentManager.getComponent<components::Sprite>(entity);
                            width = sprite.width;
                            height = sprite.height;
                        }

                        const float halfWidth = std::max(width * 0.5f, 0.01f);
                        const float halfHeight = std::max(height * 0.5f, 0.01f);

                        b2Polygon box = b2MakeBox(halfWidth, halfHeight);
                        b2CreatePolygonShape(physics.bodyId, &shapeDef, &box);
                    }
                }

                if (B2_IS_NON_NULL(physics.bodyId) && hasVelocity)
                {
                    b2Vec2 currentVelocity = b2Body_GetLinearVelocity(physics.bodyId);

                    // Horizontal motion is input-driven.
                    currentVelocity.x = desiredHorizontalVelocity;

                    // Vertical motion is physics-driven. Treat positive vy as a
                    // one-shot jump request and only consume it when near grounded speed.
                    if (desiredVerticalVelocity > 0.0f && std::abs(currentVelocity.y) < 0.01f)
                    {
                        currentVelocity.y = desiredVerticalVelocity;
                        velocity->vy = 0.0f;
                    }

                    b2Body_SetLinearVelocity(physics.bodyId, currentVelocity);
                }
            }

            _boxWorld->step(static_cast<float>(dt), 4);

            for (auto entity : entities)
            {
                auto& transform = componentManager.getComponent<components::Transform>(entity);
                auto& physics = componentManager.getComponent<components::PhysicsBody>(entity);

                if (!physics.enabled || B2_IS_NULL(physics.bodyId))
                {
                    continue;
                }

                const b2Transform bodyTransform = b2Body_GetTransform(physics.bodyId);
                transform.prevX = transform.x;
                transform.prevY = transform.y;
                transform.x = bodyTransform.p.x;
                transform.y = bodyTransform.p.y;
                transform.rotation = b2Rot_GetAngle(bodyTransform.q);
            }
        }

    private:
        engine::physics::BoxWorld* _boxWorld = nullptr;
        std::unordered_map<ecs::EntityID, b2BodyId> _bodyByEntity;
    };
} // namespace ecs::systems