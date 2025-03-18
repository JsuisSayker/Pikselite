#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP

#include "ASystem.hpp"
#include <iostream>

class InputSystem : public ASystem {
public:
    virtual ~InputSystem() = default;

    void update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events) override;
};

#endif // INPUT_SYSTEM_HPP
