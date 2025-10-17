#pragma once

#include <chrono>

namespace engine
{
    class Timer
    {
    public:
        void tick();

        float getDeltaTime() const;

    private:
        std::chrono::high_resolution_clock::time_point last = std::chrono::high_resolution_clock::now();
        float deltaTime = 0.0f;
    };
} // namespace engine