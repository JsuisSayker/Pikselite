#pragma once

#include <graphics/renderer/renderer.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/imgui/imguiInterface.hpp>
#include <engine/pixels/chunk.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <engine/ecs/components/gameObjectComponent.hpp>
#include <limits>
#include <cmath>
#include <unordered_map>

namespace editors {

    class SpriteEditor {
    public:
        SpriteEditor(
            graphics::Interface* graphicsInterface,
            graphics::Renderer* renderer,
            graphics::ImguiInterface* imguiInterface);
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

        Pixel::DefaultPixelProperties _defaultPixelProperties;

        void handleEvents(const graphics::InputEvent& event);
        void mouseLeftClick();
        void mouseLeftDrag();
        void applyBrushAt(glm::vec2 worldPos, bool erase);

        glm::vec2 screenToWorld(glm::vec2 screenPos);
        graphics::Pixel* getPixelAt(glm::vec2 worldPos);
        bool removePixelAt(glm::vec2 worldPos);
        void addPixel(glm::vec2 worldPos, Pixel::DefaultPixelProperties defaultProperties);

        void imguiHandling();

        bool saveSpriteToFile(const std::string& filename);
        bool loadSpriteFromFile(const std::string& filename);

        void removePixelAttributes(Pixel::PixelEntityID id);

        // Imgui state
        graphics::Pixel* _currentPixel = nullptr;

        bool _showPixelEditor = false;
        bool _showDefaultPropertiesEditor = false;
        bool _isEraserActive = false;
        int _selectedTool = 0; // 0 = paint, 1 = eraser
        int _brushSize = 1;

        std::string _currentSpriteFilename;
        std::string _newSpritePath;

        struct PendingCell {
            int localGX = 0;
            int localGY = 0;
            Pixel::PixelEntityID oldId = Pixel::EMPTY;
        };

        struct PendingSprite {
            bool valid = false;
            std::vector<PendingCell> cells;
            std::vector<graphics::Pixel> previewLocalPixels;
            std::unordered_map<Pixel::PixelEntityID, int> oldRenderIndex;
            std::unordered_map<Pixel::PixelEntityID, Pixel::Solid> solids;
            std::unordered_map<Pixel::PixelEntityID, Pixel::Liquid> liquids;
            std::unordered_map<Pixel::PixelEntityID, Pixel::Gaseous> gases;
            std::vector<graphics::Pixel> loadedRenderPixels;
        };

        PendingSprite _pendingSprite;
        bool _isPlacingSprite = false;

        bool loadSpriteForPlacement(const std::string& filename);
        void placePendingSpriteAtWorld(glm::vec2 worldPos);
    };
} // namespace editors
