#pragma once

#include <graphic/GraphicEnum.hpp>
#include <utils/Clock.hpp>


class ISystem
{
public:
    virtual ~ISystem() = default;

    virtual void update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) = 0;

    virtual void restartTimer() = 0;

    protected:
        Clock _timer;
        float _time = 0.5;
};
