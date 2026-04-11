#pragma once

#include <vector>

#include <box2d/box2d.h>

#include "engine/ecs/ISystem.hpp"
#include "engine/managers/componentManager.hpp"
#include "engine/physics/boxWorld.hpp"
#include "engine/ecs/components/transformComponent.hpp"
#include "engine/ecs/components/physicsComponent.hpp"

namespace ecs::systems
{
    class PhysicsSystem : public ISystem
    {
    public:
        explicit PhysicsSystem(engine::physics::BoxWorld* boxWorld)
            : _boxWorld(boxWorld) {}

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
            if (!_boxWorld || !_boxWorld->isValid() || entities.empty())
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
                        }
                    }
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