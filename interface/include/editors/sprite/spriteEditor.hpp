#pragma once

#include <graphics/renderer/renderer.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/imgui/imguiInterface.hpp>
#include <iostream>

namespace editors {
    class SpriteEditor {
    public:
        SpriteEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface);
        ~SpriteEditor();

        void run(graphics::InputEventType eventType);

    private:
        graphics::ImguiInterface* _imguiInterface;
        graphics::Interface* _graphicsInterface;
        graphics::Renderer* _renderer;

        graphics::Camera2D _camera;

        std::vector<graphics::Pixel> _spritePixels;

        void handleEvents(graphics::InputEventType eventType);
        void mouseLeftClick();

        void addPixel(float mx, float my, float r, float g, float b);
    };
} // namespace editors
