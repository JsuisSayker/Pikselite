#pragma once

#include <graphic/Graphic.hpp>

#include <memory>

// the core is the engine that save entity and manage the game loop

class Core {
    public:
        Core(std::shared_ptr<graphic::Graphic> graphic);
        ~Core();

        std::vector<std::pair<std::string, std::vector<graphic::Pixel>>> getSprite() { return _Sprites; }

        void addSprite(std::pair<std::string, std::vector<graphic::Pixel>> sprite);

    protected:
    private:
        std::shared_ptr<graphic::Graphic> _graphic;
        std::vector<std::pair<std::string, std::vector<graphic::Pixel>>> _Sprites;
};
