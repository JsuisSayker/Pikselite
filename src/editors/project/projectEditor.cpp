#include <editors/project/projectEditor.hpp>

namespace editors {

    ProjectEditor::ProjectEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface)
        : _graphicsInterface(graphicsInterface), _renderer(renderer), _imguiInterface(imguiInterface) {}

    ProjectEditor::~ProjectEditor() {}

    void ProjectEditor::run(graphics::InputEventType eventType) {
        handleEvents(eventType);

        _renderer->clear();

        // ImGui part
        _imguiInterface->startFrame();

        _renderer->drawPixelsWCamera(_renderPixels, _camera, PIXEL_SIZE);
        _renderer->drawGrid(_camera, PIXEL_SIZE, {0.7f, 0.7f, 0.7f}); // Draw grid with cell size 16

        _imguiInterface->showImGuiDemo();
        _imguiInterface->endFrame(_graphicsInterface->getWindow());

        _renderer->present(_graphicsInterface->getWindow());
    }

    void ProjectEditor::handleEvents(graphics::InputEventType eventType) {
        switch (eventType) {
        case graphics::KEY_W:
            _camera.move(glm::vec2(0.0f, -10.0f));
            break;
        case graphics::KEY_S:
            _camera.move(glm::vec2(0.0f, 10.0f));
            break;
        case graphics::KEY_A:
            _camera.move(glm::vec2(10.0f, 0.0f));
            break;
        case graphics::KEY_D:
            _camera.move(glm::vec2(-10.0f, 0.0f));
            break;
        case graphics::KEY_I:
            _camera.zoomIn(1.1f);
            break;
        case graphics::KEY_O:
            _camera.zoomOut(1.1f);
            break;
        case graphics::KEY_L:
            loadSpriteFromFile("sprite.dat");
            break;
        default:
            break;
        }
    }

    bool ProjectEditor::loadSpriteFromFile(const std::string& filename) {
        std::ifstream fin(filename, std::ios::binary);
        if (!fin) return false;
        _renderPixels.clear();

        uint32_t numChunks = 0;
        Pixel::PixelSprite newSprite;
        std::unordered_map<Pixel::PixelEntityID, Pixel::PixelEntityID> pixelIds;

        newSprite.id = spriteIdCounter++;
        fin.read(reinterpret_cast<char*>(&numChunks), sizeof(numChunks));

        for (uint32_t i = 0; i < numChunks; ++i)
        {
            int32_t cx = 0;
            int32_t cy = 0;
            fin.read(reinterpret_cast<char*>(&cx), sizeof(cx));
            fin.read(reinterpret_cast<char*>(&cy), sizeof(cy));

            for (int x = 0; x < Pixel::CHUNK_SIZE; ++x) {
                for (int y = 0; y < Pixel::CHUNK_SIZE; ++y) {
                    Pixel::PixelEntityID id = Pixel::EMPTY;
                    fin.read(reinterpret_cast<char*>(&id), sizeof(id));
                    if (id != Pixel::EMPTY) {
                        pixelIds[id] = pixelIdCounter++;
                        newSprite.pixelEntities.push_back(pixelIds[id]);
                    }
                }
            }
        }
        _sprites.push_back(newSprite);

        uint32_t count = 0;
        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            int index = 0;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            fin.read(reinterpret_cast<char*>(&index), sizeof(index));
            _pixelAttributes.renderIndex[pixelIds[id]] = index;
        }

        count = 0;
        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            _pixelAttributes.solidAttributes[pixelIds[id]] = Pixel::Solid();
        }

        count = 0;
        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            float viscosity = 0.0f;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            fin.read(reinterpret_cast<char*>(&viscosity), sizeof(viscosity));
            _pixelAttributes.liquidAttributes[pixelIds[id]] = Pixel::Liquid{viscosity};
        }

        count = 0;
        fin.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (uint32_t i = 0; i < count; ++i) {
            Pixel::PixelEntityID id = Pixel::EMPTY;
            float density = 0.0f;
            fin.read(reinterpret_cast<char*>(&id), sizeof(id));
            fin.read(reinterpret_cast<char*>(&density), sizeof(density));
            _pixelAttributes.gaseousAttributes[pixelIds[id]] = Pixel::Gaseous{density};
        }

        uint32_t numPixels = 0;
        fin.read(reinterpret_cast<char*>(&numPixels), sizeof(numPixels));
        for (uint32_t i = 0; i < numPixels; ++i) {
            float px = 0.0f;
            float py = 0.0f;
            float r = 0.0f;
            float g = 0.0f;
            float b = 0.0f;

            fin.read(reinterpret_cast<char*>(&px), sizeof(px));
            fin.read(reinterpret_cast<char*>(&py), sizeof(py));
            fin.read(reinterpret_cast<char*>(&r), sizeof(r));
            fin.read(reinterpret_cast<char*>(&g), sizeof(g));
            fin.read(reinterpret_cast<char*>(&b), sizeof(b));

            _renderPixels.push_back({{px, py}, {r, g, b}});
        }

        fin.close();
        pixelIdCounter = _renderPixels.size() + 1;
        return true;
    }

} // namespace editors