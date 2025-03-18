#ifndef MOVEMENT_SYSTEM_HPP
#define MOVEMENT_SYSTEM_HPP

#include "ASystem.hpp"

#include <algorithm>
#include <iostream>

class MovementSystem : public ASystem {
public:
    virtual ~MovementSystem() = default;

    void update(float deltaTime, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) override;
};

#endif // MOVEMENT_SYSTEM_HPP
