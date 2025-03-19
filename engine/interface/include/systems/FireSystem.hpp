#pragma once

#include <systems/ASystem.hpp>

#include <utils/Clock.hpp>

class FireSystem : public ASystem
{
public:
    virtual ~FireSystem() = default;

    void update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) override;

protected:
    Clock _timer;
    float _time = 1.0f;
};