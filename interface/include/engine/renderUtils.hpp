#pragma once

#include <engine/pixels/pixelEnum.hpp>
#include <engine/pixels/simulation/chunk.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <graphics/renderer/renderer.hpp>
#include <vector>

namespace engine
{
    std::vector<graphics::Pixel> buildRenderPixels(const ChunkGrid& grid);
}