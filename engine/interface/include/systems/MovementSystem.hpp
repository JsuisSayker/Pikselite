#ifndef MOVEMENT_SYSTEM_HPP
#define MOVEMENT_SYSTEM_HPP

#include "ASystem.hpp"
#include <iostream>

class MovementSystem : public ASystem {
public:
    virtual ~MovementSystem() = default;

    void update(float deltaTime) override;
};

#endif // MOVEMENT_SYSTEM_HPP
