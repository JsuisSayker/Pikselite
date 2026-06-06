#pragma once

#include <cmath>
#include <engine/pixels/simulation/chunk.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <fstream>
#include <graphics/imgui/imguiInterface.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/renderer/renderer.hpp>
#include <limits>
#include <string>
#include <vector>

namespace editors
{

    class AbstractEditor
    {
      public:
        virtual ~AbstractEditor() = default;
        virtual void run(const graphics::InputEvent& event) = 0;
        graphics::Camera2D getCamera() const
        {
            return _camera;
        }

        void setCamera(const graphics::Camera2D& cam)
        {
            _camera = cam;
        }

      protected:
        struct PendingCell
        {
            int localGX = 0;
            int localGY = 0;
            Element::ElementType type = Element::ElementType::EMPTY;
        };

        struct PendingSprite
        {
            bool valid = false;
            std::string sourceDatPath;
            std::vector<PendingCell> cells;
        };

        AbstractEditor(graphics::Interface* gi, graphics::Renderer* r,
                       graphics::ImguiInterface* ii);

        glm::vec2 screenToWorld(const glm::vec2& screenPos) const;
        void zoomAroundMouse(float factor);
        void panCameraScreenDelta(const glm::vec2& screenDelta);
        bool loadSpriteForPlacement(const std::string& filename);
        std::vector<graphics::Pixel> addPendingSpriteToRenderPixels();
        std::vector<graphics::Pixel> buildRenderPixels() const;

        static int toChunk(int g);
        static int toLocal(int g);

        graphics::ImguiInterface* _imguiInterface = nullptr;
        graphics::Interface* _graphicsInterface = nullptr;
        graphics::Renderer* _renderer = nullptr;

        graphics::Camera2D _camera;
        ChunkGrid _chunkGrid;
        std::vector<graphics::Pixel> _renderPixels;

        PendingSprite _pendingSprite;
        bool _isPlacingSprite = false;
        std::string _currentSpriteFilename;
    };

} // namespace editors