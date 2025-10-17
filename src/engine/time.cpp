#include "engine/time.hpp"

namespace engine
{
    void Timer::tick()
    {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();
        deltaTime = duration<float>(now - last).count();
        last = now;
    }

    float Timer::getDeltaTime() const { return deltaTime; }

} // namespace engine