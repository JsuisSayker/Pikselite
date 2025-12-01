    #pragma once

    #include <graphics/graphicsEnum.hpp>
    #include <unordered_map>
    #include <cstdint>

    namespace Pixel {
        using PixelEntityID = std::uint32_t;

        static constexpr PixelEntityID EMPTY = 0;

        struct PixelEntity {
            PixelEntityID id;
            std::uint32_t renderIndex = 0;
        };

        struct Solid {};
        struct Liquid {
            float viscosity;
        };
        struct Gaseous {
            float density;
        };

        struct PixelAttributes {
            std::unordered_map<PixelEntityID, Solid> solidAttributes;
            std::unordered_map<PixelEntityID, Liquid> liquidAttributes;
            std::unordered_map<PixelEntityID, Gaseous> gaseousAttributes;
        };

    }
