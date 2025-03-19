#ifndef MOVEMENT_SYSTEM_HPP
#define MOVEMENT_SYSTEM_HPP

#include "ASystem.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

class MovementSystem : public ASystem {
public:
    MovementSystem();
    ~MovementSystem();

    void update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) override;

    private:
        Clock _clock;
        graphic::EventType _event = graphic::EventType::NONE;
};

#endif // MOVEMENT_SYSTEM_HPP
