#include <core/Core.hpp>

Core::Core(std::shared_ptr<graphic::Graphic> graphic) : _graphic(graphic)
{
}

Core::~Core()
{
}

void Core::addSprite(std::pair<std::string, std::vector<graphic::Pixel>> sprite)
{
    this->_Sprites.push_back(sprite);
}