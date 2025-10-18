#pragma once

#include <graphics/renderer/renderer.hpp>
#include <graphics/interface/interface.hpp>
#include <iostream>

namespace editors {
    class SpriteEditor {
    public:
        SpriteEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer);
        ~SpriteEditor();

        void run(graphics::InputEventType eventType);

    private:
        graphics::Interface* _graphicsInterface;
        graphics::Renderer* _renderer;

        std::vector<graphics::Pixel> _spritePixels;

        void handleEvents(graphics::InputEventType eventType);
        void mouseLeftClick();

        void addPixel(float x, float y, float r, float g, float b);
    };
} // namespace editors
