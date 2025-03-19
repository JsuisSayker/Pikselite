#include <systems/FireSystem.hpp>
#include <unistd.h>
#include <cmath>
#include <chrono>
#include <cstdint>

graphic::Pixel *findPixelAt(std::vector<graphic::Sprite> &sprites, int x, int y)
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
}

graphic::Color mixColors(const graphic::Color &c1, const graphic::Color &c2, float factor)
{
    graphic::Color result;
    result.r = static_cast<uint8_t>(c1.r + factor * (c2.r - c1.r));
    result.g = static_cast<uint8_t>(c1.g + factor * (c2.g - c1.g));
    result.b = static_cast<uint8_t>(c1.b + factor * (c2.b - c1.b));
    result.a = static_cast<uint8_t>(c1.a + factor * (c2.a - c1.a));
    return result;
}

void FireSystem::update(Clock clock, std::vector<graphic::Sprite> &sprites, std::vector<graphic::EventType> events)
{
    auto now = std::chrono::steady_clock::now();
    float timeInSeconds = std::chrono::duration<float>(now.time_since_epoch()).count();
    float frequency = 30.0f;

    graphic::Color fireColor = {255, 69, 0, 255};
    graphic::Color fireColor2 = {255, 165, 0, 255};

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
            if (isOnFire)
            {
                float flicker = (std::sin(timeInSeconds * frequency + pixel.position.x * 0.1f + pixel.position.y * 0.1f) + 1.0f) / 2.0f;
                pixel.color = mixColors(fireColor, fireColor2, flicker);
            }
        }
    }

    if (_timer.getElapsedTime() < _time)
        return;

    _timer.restart();

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

            for (const auto &dir : directions)
            {
                int nx = pixel.position.x + dir.first;
                int ny = pixel.position.y + dir.second;

                graphic::Pixel *neighbor = findPixelAt(sprites, nx, ny);
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
                }
            }
        }
    }
}
