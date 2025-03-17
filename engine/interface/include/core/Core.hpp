#pragma once

#include <graphic/Graphic.hpp>
#include <systems/ASystem.hpp>

#include <memory>

// the core is the engine that save entity and manage the game loop

class Core {
    public:
        Core(std::shared_ptr<graphic::Graphic> graphic);
        ~Core();

        std::vector<graphic::Sprite> getSprite() { return _Sprites; }

        void addSprite(graphic::Sprite sprite);

    protected:
    private:
        std::shared_ptr<graphic::Graphic> _graphic;
        std::vector<graphic::Sprite> _Sprites;
};
