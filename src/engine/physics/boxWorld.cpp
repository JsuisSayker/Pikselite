#include "engine/physics/boxWorld.hpp"

namespace engine::physics
{

    BoxWorld::~BoxWorld()
    {
        shutdown();
    }

    void BoxWorld::init(b2Vec2 gravity)
    {
        if (B2_IS_NON_NULL(worldId))
        {
            return;
        }

        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = gravity;
        worldId = b2CreateWorld(&worldDef);
    }

    void BoxWorld::shutdown()
    {
        if (B2_IS_NON_NULL(worldId))
        {
            b2DestroyWorld(worldId);
            worldId = b2_nullWorldId;
        }
    }

    void BoxWorld::step(float timeStep, int subStepCount)
    {
        if (B2_IS_NON_NULL(worldId))
        {
            b2World_Step(worldId, timeStep, subStepCount);
        }
    }

    [[nodiscard]] b2WorldId BoxWorld::getWorldId() const
    {
        return worldId;
    }

    [[nodiscard]] bool BoxWorld::isValid() const
    {
        return B2_IS_NON_NULL(worldId);
    }
} // namespace engine::physics