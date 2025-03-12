#pragma once

#include <memory>
#include <graphic/Graphic.hpp>

class SpriteEditor {
    public:
        SpriteEditor(std::shared_ptr<graphic::Graphic> graphic);
        ~SpriteEditor();

        int run();
        void moveCamera(graphic::EventType event);
        void zoomCamera(graphic::EventType event);
        void checkMouseEvents(graphic::EventType event);
        void checkInterfaceEvents(graphic::EventType event);

        void addPixel(graphic::Pixel pixel);

    protected:
    private:
        graphic::Camera _camera = graphic::Camera{WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 4};
        std::shared_ptr<graphic::Graphic> _graphic;
};
