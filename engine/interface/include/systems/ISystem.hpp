#pragma once

#include <graphic/GraphicEnum.hpp>


class ISystem
{
public:
    virtual ~ISystem() = default;

    virtual void update(float deltaTime, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) = 0;
};
