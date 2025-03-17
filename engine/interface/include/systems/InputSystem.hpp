#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP

#include "ASystem.hpp"
#include <iostream>

class InputSystem : public ASystem {
public:
    virtual ~InputSystem() = default;

    void update(float deltaTime, std::vector<graphic::Sprite> sprites) override;
};

#endif // INPUT_SYSTEM_HPP
