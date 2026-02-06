#pragma once

#include <graphics/graphicsEnum.hpp>
#include <unordered_map>
#include <cstdint>

namespace Pixel {
    using PixelEntityID = std::uint32_t;
    using GameObjectID = std::uint32_t;

    static constexpr PixelEntityID EMPTY = 0;
    static constexpr GameObjectID NO_SPRITE = 0;

    struct Solid {};
    struct Liquid {
        float viscosity;
    };
    struct Gaseous {
        float density;
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
    };
}
