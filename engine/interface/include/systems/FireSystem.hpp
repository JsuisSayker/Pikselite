#pragma once

#include <systems/ASystem.hpp>

#include <unistd.h>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <algorithm>

#include <utils/Clock.hpp>

class FireSystem : public ASystem
{
public:
    virtual ~FireSystem() = default;

    void update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) override;

protected:
};