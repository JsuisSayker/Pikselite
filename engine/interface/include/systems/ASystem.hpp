#ifndef ASYSTEM_HPP
#define ASYSTEM_HPP

#include "ISystem.hpp"

class ASystem : public ISystem {
public:
    virtual ~ASystem() = default;

    virtual void update(float deltaTime, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) = 0;
};

#endif // ASYSTEM_HPP
