#include <systems/SandSystem.hpp>

SandSystem::SandSystem()
{
}

SandSystem::~SandSystem()
{
}

void SandSystem::update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events)
{
    if (_timer.getElapsedTime() < 0.03)
        return;

    bool canMoveDown = true;
    bool canMoveDLeft = true;
    bool canMoveRight = true;
    _timer.restart();
    for (graphic::Sprite &sprite : sprites)
    {
        for (graphic::Pixel &pixel : sprite.pixels)
        {
            canMoveDown = true;
            canMoveDLeft = true;
            canMoveRight = true;
            if (pixel.attributes.empty())
            {
                std::cout << "No attributes" << std::endl;
                continue;
            }

            for (std::variant<graphic::light, graphic::solid, graphic::liquid, graphic::fire, graphic::flammable, graphic::willBurn, graphic::sand, graphic::burned> &attribute : pixel.attributes)
            {
                if (std::holds_alternative<graphic::sand>(attribute))
                {
                    graphic::sand sand = std::get<graphic::sand>(attribute);
                    for (graphic::Pixel &otherPixel : sprite.pixels)
                    {
                        if (pixel.position.x == otherPixel.position.x && pixel.position.y + 1 == otherPixel.position.y)
                        {
                            canMoveDown = false;
                        }
                    }
                    if (!canMoveDown)
                    {
                        for (graphic::Pixel &otherPixel : sprite.pixels)
                        {
                            if (pixel.position.x - 1 == otherPixel.position.x && pixel.position.y + 1 == otherPixel.position.y)
                            {
                                canMoveDLeft = false;
                            }
                            if (pixel.position.x + 1 == otherPixel.position.x && pixel.position.y + 1 == otherPixel.position.y)
                            {
                                canMoveRight = false;
                            }
                        }
                    }

                    if (canMoveDLeft)
                    {
                        for (graphic::Pixel &otherPixel : sprite.pixels)
                        {
                            if (pixel.position.x - 1 == otherPixel.position.x && pixel.position.y == otherPixel.position.y)
                            {
                                canMoveDLeft = false;
                            }
                        }
                    }
                    if (canMoveRight)
                    {
                        for (graphic::Pixel &otherPixel : sprite.pixels)
                        {
                            if (pixel.position.x + 1 == otherPixel.position.x && pixel.position.y == otherPixel.position.y)
                            {
                                canMoveRight = false;
                            }
                        }
                    }

                    if (canMoveDown)
                    {
                        pixel.position.y++;
                    }
                    else if (canMoveRight)
                    {
                        pixel.position.x++;
                        pixel.position.y++;
                    }
                    else if (canMoveDLeft)
                    {
                        pixel.position.x--;
                        pixel.position.y++;
                    }
                }
            }
        }
    }
}