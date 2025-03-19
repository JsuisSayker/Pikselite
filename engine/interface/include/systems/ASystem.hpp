#ifndef ASYSTEM_HPP
#define ASYSTEM_HPP

#include "ISystem.hpp"

class ASystem : public ISystem {
public:
    virtual ~ASystem() = default;

    virtual void update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) = 0;

    void restartTimer() override
    {
        _timer.restart();
    }
};

#endif // ASYSTEM_HPP
