#pragma once

#include <graphics/renderer/renderer.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/imgui/imguiInterface.hpp>
#include <iostream>

namespace editors {
    class ProjectEditor {
    public:
        ProjectEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface);
        ~ProjectEditor();

        void run(graphics::InputEventType eventType);

    private:
        graphics::ImguiInterface* _imguiInterface;
        graphics::Interface* _graphicsInterface;
        graphics::Renderer* _renderer;

        graphics::Camera2D _camera;

        void handleEvents(graphics::InputEventType eventType);
    };
} // namespace editors