#pragma once

#include <box2d/box2d.h>

namespace engine::physics
{
    class BoxWorld
    {
    public:
        BoxWorld() = default;

        explicit BoxWorld(b2Vec2 gravity)
        {
            init(gravity);
        }

        ~BoxWorld();

        void init(b2Vec2 gravity = {0.0f, -10.0f});

        void shutdown();

        void step(float timeStep, int subStepCount = 4);

        [[nodiscard]] b2WorldId getWorldId() const;

        [[nodiscard]] bool isValid() const;

    private:
        b2WorldId worldId = b2_nullWorldId;
    };
} // namespace engine::physics
