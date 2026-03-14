#pragma once

#include <graphics/renderer/renderer.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/imgui/imguiInterface.hpp>
#include <engine/pixels/chunk.hpp>
#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>
#include <unordered_map>

namespace editors {
    class ProjectEditor {
    public:
        ProjectEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface);
        ~ProjectEditor();

        void run(const graphics::InputEvent& event);

        std::vector<graphics::Pixel> getPixels() const { return _renderPixels; }
        std::vector<Pixel::GameObject> getGameObjects() const { return _gameObjects; }
        Pixel::PixelAttributes getPixelAttributes() const { return _pixelAttributes; }
        Pixel::ChunkGrid getChunkGrid() const { return _chunkGrid; }
        uint32_t getPixelIdCounter() const { return pixelIdCounter; }
        uint32_t getGameObjectCounter() const { return gameObjectCounter; }

        bool consumeSaveSceneRequest() {
            const bool requested = _saveSceneRequested;
            _saveSceneRequested = false;
            return requested;
        }

        bool consumeLoadSceneRequest() {
            const bool requested = _loadSceneRequested;
            _loadSceneRequested = false;
            return requested;
        }

        void setSceneData(const std::vector<graphics::Pixel>& renderPixels,
                          const std::vector<Pixel::GameObject>& gameObjects,
                          const Pixel::PixelAttributes& pixelAttributes,
                          const Pixel::ChunkGrid& chunkGrid,
                          uint32_t nextPixelId,
                          uint32_t nextGameObjectId);

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
        Pixel::ChunkGrid _chunkGrid;

        struct PendingCell {
            int localGX = 0;
            int localGY = 0;
            Pixel::PixelEntityID oldId = Pixel::EMPTY;
        };

        struct PendingSprite {
            bool valid = false;
            std::vector<PendingCell> cells;
            std::vector<graphics::Pixel> previewLocalPixels; // local world units (PIXEL_SIZE-based)
            std::unordered_map<Pixel::PixelEntityID, int> oldRenderIndex;
            std::unordered_map<Pixel::PixelEntityID, Pixel::Solid> solids;
            std::unordered_map<Pixel::PixelEntityID, Pixel::Liquid> liquids;
            std::unordered_map<Pixel::PixelEntityID, Pixel::Gaseous> gases;
            std::vector<graphics::Pixel> loadedRenderPixels;
        };

        PendingSprite _pendingSprite;
        bool _isPlacingSprite = false;
        bool _leftMouseDownLastFrame = false;
        bool _saveSceneRequested = false;
        bool _loadSceneRequested = false;
        std::string _currentSpriteFilename;

        void handleEvents(const graphics::InputEvent& event);
        bool loadSpriteFromFile(const std::string& filename); // keep if you still need direct load

        bool loadSpriteForPlacement(const std::string& filename);
        void updatePlacementMode();
        void placePendingSpriteAtGrid(int anchorGX, int anchorGY);
        glm::vec2 screenToWorld(const glm::vec2& screenPos) const;
    };
} // namespace editors