#ifndef ISYSTEM_HPP
#define ISYSTEM_HPP

class ISystem
{
public:
    virtual ~ISystem() = default;

    virtual void update(float deltaTime) = 0;
};

#endif // ISYSTEM_HPP
