#include <editors/aEditor.hpp>
#include <SDL2/SDL.h>

namespace editors {

AbstractEditor::AbstractEditor(graphics::Interface* gi, graphics::Renderer* r, graphics::ImguiInterface* ii)
    : _imguiInterface(ii), _graphicsInterface(gi), _renderer(r) {}

int AbstractEditor::toChunk(int g) {
    return (g >= 0) ? (g / CHUNK_SIZE) : ((g - CHUNK_SIZE + 1) / CHUNK_SIZE);
}

int AbstractEditor::toLocal(int g) {
    return ((g % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
}

glm::vec2 AbstractEditor::screenToWorld(const glm::vec2& screenPos) const {
    int w = 0, h = 0;
    SDL_GetWindowSize(_graphicsInterface->getWindow(), &w, &h);
    return _camera.screenToWorld(screenPos, w, h);
}

bool AbstractEditor::loadSpriteForPlacement(const std::string& filename) {
    _pendingSprite = {};
    _isPlacingSprite = false;

    std::ifstream fin(filename, std::ios::binary);
    if (!fin) return false;

    PendingSprite pending{};
    int minGX = std::numeric_limits<int>::max();
    int minGY = std::numeric_limits<int>::max();
    std::vector<std::pair<std::pair<int32_t, int32_t>, std::vector<std::pair<int, Element::ElementType>>>> allChunks;

    uint32_t nbChunks = 0;
    fin.read(reinterpret_cast<char*>(&nbChunks), sizeof(nbChunks));

    for (uint32_t i = 0; i < nbChunks; ++i) {
        int32_t cx = 0, cy = 0;
        fin.read(reinterpret_cast<char*>(&cx), sizeof(cx));
        fin.read(reinterpret_cast<char*>(&cy), sizeof(cy));

        std::vector<std::pair<int, Element::ElementType>> pixelsInChunk;
        for (int j = 0; j < CHUNK_SIZE * CHUNK_SIZE; ++j) {
            Element::ElementType type = Element::EMPTY;
            fin.read(reinterpret_cast<char*>(&type), sizeof(type));
            if (type == Element::EMPTY) continue;

            int lx = j % CHUNK_SIZE;
            int ly = j / CHUNK_SIZE;
            int gx = cx * CHUNK_SIZE + lx;
            int gy = cy * CHUNK_SIZE + ly;
            minGX = std::min(minGX, gx);
            minGY = std::min(minGY, gy);
            pixelsInChunk.push_back({j, type});
        }
        allChunks.push_back({{cx, cy}, std::move(pixelsInChunk)});
    }

    for (const auto& [coords, pixelsInChunk] : allChunks) {
        const int32_t cx = coords.first;
        const int32_t cy = coords.second;
        for (const auto& [j, type] : pixelsInChunk) {
            int lx = j % CHUNK_SIZE;
            int ly = j / CHUNK_SIZE;
            int gx = cx * CHUNK_SIZE + lx;
            int gy = cy * CHUNK_SIZE + ly;
            pending.cells.push_back({gx - minGX, gy - minGY, type});
        }
    }

    pending.valid = !pending.cells.empty();
    _pendingSprite = std::move(pending);
    return _pendingSprite.valid;
}

std::vector<graphics::Pixel> AbstractEditor::addPendingSpriteToRenderPixels() {
    std::vector<graphics::Pixel> out;
    if (!_pendingSprite.valid) return out;

    glm::vec2 mousePos = _graphicsInterface->getMousePosition();
    glm::vec2 worldPos = screenToWorld(mousePos);
    int anchorGX = static_cast<int>(std::floor(worldPos.x / PIXEL_SIZE));
    int anchorGY = static_cast<int>(std::floor(worldPos.y / PIXEL_SIZE));

    out.reserve(_pendingSprite.cells.size());
    for (const auto& cell : _pendingSprite.cells) {
        graphics::Pixel p;
        p.position = {
            (anchorGX + cell.localGX) * PIXEL_SIZE,
            (anchorGY + cell.localGY) * PIXEL_SIZE
        };
        const auto& def = g_elements[cell.type];
        p.color = {
            def.color[0] / 255.0f,
            def.color[1] / 255.0f,
            def.color[2] / 255.0f
        };
        p.color *= 0.65f;
        out.push_back(p);
    }
    return out;
}

std::vector<graphics::Pixel> AbstractEditor::buildRenderPixels() const {
    std::vector<graphics::Pixel> result;
    for (const auto& [key, chunk] : _chunkGrid.chunks) {
        const int cx = static_cast<int32_t>(key >> 32);
        const int cy = static_cast<int32_t>(key & 0xFFFFFFFF);

        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                const Element::Pixel& sp = chunk.pixels[y * CHUNK_SIZE + x];
                if (sp.type == Element::EMPTY) continue;

                const auto& def = g_elements[sp.type];
                graphics::Pixel rp;
                rp.position = {
                    (cx * CHUNK_SIZE + x) * PIXEL_SIZE,
                    (cy * CHUNK_SIZE + y) * PIXEL_SIZE
                };
                rp.color = {
                    def.color[0] / 255.0f,
                    def.color[1] / 255.0f,
                    def.color[2] / 255.0f
                };
                result.push_back(rp);
            }
        }
    }
    return result;
}

} // namespace editors