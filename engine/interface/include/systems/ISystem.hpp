#pragma once

#include <graphic/GraphicEnum.hpp>
#include <utils/Clock.hpp>


class ISystem
{
public:
    virtual ~ISystem() = default;

    virtual void update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) = 0;
};
