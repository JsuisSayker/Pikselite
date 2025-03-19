#include <systems/FireSystem.hpp>
#include <unistd.h>

void FireSystem::update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events)
{
    if (_timer.getElapsedTime() < 1)
        return;

    std::cout << "Timer before :" << _timer.getElapsedTime() << std::endl;

    _timer.restart();

    std::cout << "Timer after :" << _timer.getElapsedTime() << std::endl;

    graphic::Color fireColor = {255, 69, 0, 255};

    auto findPixelAt = [&sprites](int x, int y) -> graphic::Pixel *
    {
        for (auto &sprite : sprites)
        {
            for (auto &pix : sprite.pixels)
            {
                if (pix.position.x == x && pix.position.y == y)
                    return &pix;
            }
        }
        return nullptr;
    };

    const std::vector<std::pair<int, int>> directions = {
        {0, -1}, {-1, 0}, {1, 0}, {0, 1}};

    for (auto &sprite : sprites)
    {
        for (graphic::Pixel &pixel : sprite.pixels)
        {
            bool isOnFire = false;
            for (auto &attribute : pixel.attributes)
            {
                if (std::holds_alternative<graphic::fire>(attribute))
                {
                    isOnFire = true;
                    break;
                }
            }
            if (!isOnFire)
                continue;

            std::cout << "| ";

            pixel.color = fireColor;

            for (const auto &dir : directions)
            {
                int nx = pixel.position.x + dir.first;
                int ny = pixel.position.y + dir.second;

                graphic::Pixel *neighbor = findPixelAt(nx, ny);
                if (neighbor)
                {
                    bool flammable = false;
                    bool neighborOnFire = false;
                    bool alreadyWillBurn = false;
                    for (auto &nAttr : neighbor->attributes)
                    {
                        if (std::holds_alternative<graphic::flammable>(nAttr))
                            flammable = true;
                        if (std::holds_alternative<graphic::fire>(nAttr))
                            neighborOnFire = true;
                        if (std::holds_alternative<graphic::willBurn>(nAttr))
                            alreadyWillBurn = true;
                    }
                    if (flammable && !neighborOnFire && !alreadyWillBurn)
                    {
                        neighbor->attributes.push_back(graphic::willBurn{});
                    }
                }
            }
        }
    }

    for (auto &sprite : sprites)
    {
        for (graphic::Pixel &pixel : sprite.pixels)
        {
            bool hasWillBurn = false;
            for (auto it = pixel.attributes.begin(); it != pixel.attributes.end();)
            {
                if (std::holds_alternative<graphic::willBurn>(*it))
                {
                    hasWillBurn = true;
                    it = pixel.attributes.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            if (hasWillBurn)
            {
                bool alreadyFire = false;
                for (auto &attribute : pixel.attributes)
                {
                    if (std::holds_alternative<graphic::fire>(attribute))
                    {
                        alreadyFire = true;
                        break;
                    }
                }
                if (!alreadyFire)
                {
                    pixel.attributes.push_back(graphic::fire{});
                    pixel.color = fireColor;
                }
            }
        }
    }
}
