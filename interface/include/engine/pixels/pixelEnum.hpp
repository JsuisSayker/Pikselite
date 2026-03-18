#pragma once

#include <graphics/graphicsEnum.hpp>
#include <unordered_map>
#include <cstdint>
#include <string>

namespace Pixel {
    using PixelEntityID = std::uint32_t;
    using GameObjectID = std::uint32_t;

    static constexpr PixelEntityID EMPTY = 0;
    static constexpr GameObjectID NO_SPRITE = 0;

    struct Solid {};
    struct Liquid {
        float viscosity;
        bool updateThisFrame = false;
    };
    struct Gaseous {
        float density;
        bool updateThisFrame = false;
    };

    struct PixelAttributes {
        std::unordered_map<PixelEntityID, int> renderIndex;
        std::unordered_map<PixelEntityID, Solid> solidAttributes;
        std::unordered_map<PixelEntityID, Liquid> liquidAttributes;
        std::unordered_map<PixelEntityID, Gaseous> gaseousAttributes;
    };

    struct GameObject {
        GameObjectID id;
        std::vector<PixelEntityID> pixelEntities;
        std::string name;
    };

    struct DefaultPixelProperties {
        glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
        bool isSolid = false;
        bool isLiquid = false;
        bool isGaseous = false;
        Solid solidAttributes;
        Liquid liquidAttributes{0.5f};
        Gaseous gaseousAttributes{0.5f};
    };
}
