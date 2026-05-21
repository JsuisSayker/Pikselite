#include <engine/renderUtils.hpp>
#include <glm/common.hpp>
#include <glm/glm.hpp>

namespace engine
{
    std::vector<graphics::Pixel> buildRenderPixels(const ChunkGrid& grid)
    {
        std::vector<graphics::Pixel> result;
        result.reserve(10000);

        for (const auto& [key, chunk] : grid.chunks)
        {
            // Correct signed decode from packed int64 key
            const int cx = static_cast<int32_t>(key >> 32);
            const int cy = static_cast<int32_t>(key & 0xFFFFFFFF);

            for (int y = 0; y < CHUNK_SIZE; ++y)
            {
                for (int x = 0; x < CHUNK_SIZE; ++x)
                {
                    const Element::Pixel& simPixel = chunk.pixels[y * CHUNK_SIZE + x];
                    if (simPixel.type == Element::EMPTY)
                        continue;

                    const auto& def = g_elements[simPixel.type];

                    graphics::Pixel renderPixel;

                    // grid -> world (apply chunk offset + pixel size)
                    const float gx = static_cast<float>(cx * CHUNK_SIZE + x);
                    const float gy = static_cast<float>(cy * CHUNK_SIZE + y);

                    renderPixel.position = glm::vec2(gx * PIXEL_SIZE, gy * PIXEL_SIZE);

                    if (simPixel.isBurning)
                    {
                        glm::vec3 pColor = glm::vec3(
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].r / 255.0f,
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].g / 255.0f,
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].b / 255.0f);
                        ElementDefinition& fireDef = g_elements[Element::FIRE];
                        glm::vec3 fColor = glm::vec3(
                            fireDef.colorPalette[simPixel.colorIndex % PALETTE_SIZE].r / 255.0f,
                            fireDef.colorPalette[simPixel.colorIndex % PALETTE_SIZE].g / 255.0f,
                            fireDef.colorPalette[simPixel.colorIndex % PALETTE_SIZE].b / 255.0f);
                        float progress = (def.fireParams.burnDuration > 0)
                                             ? 1.0f - (static_cast<float>(simPixel.burnTimer) /
                                                       def.fireParams.burnDuration)
                                             : 1.0f;
                        progress = glm::clamp(progress, 0.0f, 1.0f);
                        renderPixel.color = glm::mix(pColor, fColor, progress);
                        renderPixel.color =
                            glm::clamp(renderPixel.color, glm::vec3(0.0f), glm::vec3(1.0f));
                    }
                    else
                    {
                        renderPixel.color = glm::vec3(
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].r / 255.0f,
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].g / 255.0f,
                            def.colorPalette[simPixel.colorIndex % PALETTE_SIZE].b / 255.0f);
                    }

                    result.push_back(renderPixel);
                }
            }
        }
        return result;
    }
} // namespace engine