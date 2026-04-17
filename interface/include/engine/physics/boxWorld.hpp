#pragma once

#include <box2d/box2d.h>

namespace engine::physics
{
    /**
     * @brief RAII wrapper around a Box2D world id.
     */
    class BoxWorld
    {
    public:
        BoxWorld() = default;

        explicit BoxWorld(b2Vec2 gravity)
        {
            init(gravity);
        }

        ~BoxWorld();

        /**
         * @brief Creates the Box2D world with the provided gravity.
         */
        void init(b2Vec2 gravity = {0.0f, -10.0f});

        /**
         * @brief Destroys the underlying Box2D world if valid.
         */
        void shutdown();

        /**
         * @brief Advances the simulation by one step.
         */
        void step(float timeStep, int subStepCount = 4);

        /**
         * @brief Returns the underlying world id.
         */
        [[nodiscard]] b2WorldId getWorldId() const;

        /**
         * @brief Returns true when the world id points to a valid Box2D world.
         */
        [[nodiscard]] bool isValid() const;

    private:
        b2WorldId worldId = b2_nullWorldId;
    };
} // namespace engine::physics
