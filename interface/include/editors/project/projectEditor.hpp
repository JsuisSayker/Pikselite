#pragma once

#include <graphics/renderer/renderer.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/imgui/imguiInterface.hpp>
#include <engine/pixels/chunk.hpp>
#include <iostream>
#include <fstream>

namespace editors {
    class ProjectEditor {
    public:
        ProjectEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface);
        ~ProjectEditor();

        void run(const graphics::InputEvent& event);

        std::vector<graphics::Pixel> getPixels() const { return _renderPixels; }
        std::vector<Pixel::GameObject> getGameObjects() const { return _gameObjects; }
        Pixel::PixelAttributes getPixelAttributes() const { return _pixelAttributes; }

    private:
        graphics::ImguiInterface* _imguiInterface;
        graphics::Interface* _graphicsInterface;
        graphics::Renderer* _renderer;

        graphics::Camera2D _camera;
        
        uint32_t pixelIdCounter = 1;
        uint32_t gameObjectCounter = 1;
        
        Pixel::PixelAttributes _pixelAttributes;

        std::vector<Pixel::GameObject> _gameObjects;
        std::vector<graphics::Pixel> _renderPixels;

        void handleEvents(const graphics::InputEvent& event);
        bool loadSpriteFromFile(const std::string& filename);
    };
} // namespace editors