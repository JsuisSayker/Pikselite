#pragma once

#include <systems/ASystem.hpp>

class SandSystem : public ASystem
{
public:
    SandSystem();
    ~SandSystem();

    void update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) override;

    private:
        Clock _clock;
};