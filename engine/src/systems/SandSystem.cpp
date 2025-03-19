#include <systems/SandSystem.hpp>

SandSystem::SandSystem()
{
}

SandSystem::~SandSystem()
{
}

void SandSystem::update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events)
{
    if (_clock.getElapsedTime() < 0.2)
        return;
    
    _clock.restart();
    for (graphic::Sprite &sprite : sprites)
    {
        for (graphic::Pixel &pixel : sprite.pixels)
        {
            if (pixel.attributes.empty())
                continue;
            
            for (std::variant<graphic::light, graphic::solid, graphic::liquid, graphic::fire, graphic::flammable, graphic::sand> &attribute : pixel.attributes)
            {
                if (std::holds_alternative<graphic::sand>(attribute))
                {
                    graphic::sand sand = std::get<graphic::sand>(attribute);
                    pixel.position.y -= 1;
                }
            }
        }
    }
}