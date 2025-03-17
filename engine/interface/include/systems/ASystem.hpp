#ifndef ASYSTEM_HPP
#define ASYSTEM_HPP

#include "ISystem.hpp"

class ASystem : public ISystem {
public:
    virtual ~ASystem() = default;

    virtual void update(float deltaTime) = 0;
};

#endif // ASYSTEM_HPP
