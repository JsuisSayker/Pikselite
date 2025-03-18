#pragma once

#include <systems/ASystem.hpp>

class FireSystem : public ASystem
{
public:
    virtual ~FireSystem() = default;

    void update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) override;
};