#pragma once

#include <graphics/renderer/renderer.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/imgui/imguiInterface.hpp>
#include <engine/pixels/chunk.hpp>
#include <iostream>
#include <fstream>

namespace editors {
    class SpriteEditor {
    public:
        SpriteEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface);
        ~SpriteEditor();

        void run(const graphics::InputEvent& event);

    private:
        graphics::ImguiInterface* _imguiInterface;
        graphics::Interface* _graphicsInterface;
        graphics::Renderer* _renderer;

        graphics::Camera2D _camera;

        uint32_t pixelIdCounter = 1;
        Pixel::ChunkGrid _chunkGrid;
        
        Pixel::PixelAttributes _pixelAttributes;
        std::vector<graphics::Pixel> _renderPixels;

        void handleEvents(const graphics::InputEvent& event);
        void mouseLeftClick();
        void mouseLeftDrag();

        glm::vec2 screenToWorld(glm::vec2 screenPos);
        graphics::Pixel* getPixelAt(glm::vec2 worldPos);
        bool removePixelAt(glm::vec2 worldPos);
        void addPixel(glm::vec2 worldPos, float r, float g, float b);

        void imguiHandling();

        bool saveSpriteToFile(const std::string& filename);
        bool loadSpriteFromFile(const std::string& filename);

        // Imgui state
        graphics::Pixel* _currentPixel = nullptr;

        bool _showPixelEditor = false;
    };
} // namespace editors
