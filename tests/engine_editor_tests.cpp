#include <gtest/gtest.h>
#include <filesystem>

#include <engine/pixels/chunk.hpp>
#include <engine/pixels/simulation/simulation.hpp>
#include <editors/project/projectEditor.hpp>
#include <editors/sprite/spriteEditor.hpp>
#include <engine/pixels/pixelEnum.hpp>
#include <graphics/imgui/components/bars.hpp>
#include <graphics/renderer/renderer.hpp>
#include <graphics/renderer/camera.hpp>
#include <tests/imguiSetupForTest.hpp>

using namespace Pixel;

// TEST(PixelAttributesTests, DefaultPropertyBehavior) {
//     Pixel::DefaultPixelProperties props;
//     EXPECT_EQ(props.color, glm::vec3(1.0f, 0.0f, 0.0f));
//     EXPECT_FALSE(props.isSolid);

//     props.isSolid = true;
//     props.solidAttributes = Pixel::Solid();
//     EXPECT_TRUE(props.isSolid);
// }

TEST(PixelSimulationTests, LiquidfallsDown) {
    ChunkGrid grid;
    PixelAttributes attributes;
    std::vector<graphics::Pixel> renderPixels;

    // Put one liquid pixel above empty space
    auto& chunk = grid.getOrCreateChunk(0, 0);
    chunk.set(0, 1, 1);
    attributes.renderIndex[1] = 0;
    attributes.liquidAttributes[1] = Pixel::Liquid{0.5f, false};
    renderPixels.push_back({glm::vec2(0, 1), glm::vec3(1.0f)});

    PixelSimulation sim;
    sim.step(grid, attributes, renderPixels, 0.02f);

    EXPECT_EQ(grid.getPixel(0, 0), 1);
    EXPECT_EQ(grid.getPixel(0, 1), EMPTY);
    EXPECT_NEAR(renderPixels[0].position.y, -9.0f, 0.01f); // PIXEL_SIZE=10.0f
}
