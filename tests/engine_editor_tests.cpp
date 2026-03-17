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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace Pixel;

TEST(ChunkTests, SetGetBoundary) {
    Chunk c;
    EXPECT_EQ(c.get(0, 0), EMPTY);
    EXPECT_EQ(c.get(CHUNK_SIZE, 0), EMPTY);

    c.set(5, 5, 123);
    EXPECT_EQ(c.get(5, 5), 123);

    c.set(-1, 0, 999);
    EXPECT_EQ(c.get(-1, 0), EMPTY);
}

TEST(ChunkGridTests, ChunkGridOperations) {
    ChunkGrid grid;
    EXPECT_EQ(grid.getPixel(0, 0), EMPTY);

    auto& chunk = grid.getOrCreateChunk(0, 0);
    chunk.set(0, 0, 42);
    EXPECT_EQ(grid.getPixel(0, 0), 42);

    EXPECT_TRUE(grid.removePixel(0, 0));
    EXPECT_EQ(grid.getPixel(0, 0), EMPTY);

    chunk.set(1, 1, 77);
    EXPECT_TRUE(grid.movePixel(1, 1, 2, 2));
    EXPECT_EQ(grid.getPixel(2, 2), 77);
    EXPECT_EQ(grid.getPixel(1, 1), EMPTY);
}

TEST(PixelAttributesTests, DefaultPropertyBehavior) {
    Pixel::DefaultPixelProperties props;
    EXPECT_EQ(props.color, glm::vec3(1.0f, 0.0f, 0.0f));
    EXPECT_FALSE(props.isSolid);

    props.isSolid = true;
    props.solidAttributes = Pixel::Solid();
    EXPECT_TRUE(props.isSolid);
}

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

TEST(GetDesiredSizeTests, NotFull) {
    EXPECT_EQ(graphics::GetDesiredSize("notfull", graphics::BarOrientation::Horizontal), 0);
    EXPECT_EQ(graphics::GetDesiredSize("notfull", graphics::BarOrientation::Vertical), 0);
}

TEST(GetDesiredSizeTests, FullReturnsDisplaySize) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    imguiTest::ImGuiTestCommon testCommon;
    testCommon.InitImGuiForTests(window, glContext);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(640, 480);

    EXPECT_EQ(graphics::GetDesiredSize("full", graphics::BarOrientation::Horizontal), 640);
    EXPECT_EQ(graphics::GetDesiredSize("full", graphics::BarOrientation::Vertical), 480);

    testCommon.ShutdownImGuiForTests(window, glContext);
}

TEST(RendererTests, BasicDrawPaths) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    imguiTest::ImGuiTestCommon testCommon;
    testCommon.InitImGuiForTests(window, glContext);

    graphics::Renderer renderer(window, glContext);

    // clear/present
    renderer.clear();
    renderer.present(window);

    // draw empty doesn't crash
    graphics::Camera2D camera;
    renderer.drawPixelsWCamera({}, camera);
    renderer.drawPixelsOverlay({}, 10.0f);

    // draw one pixel
    std::vector<graphics::Pixel> pixels{{{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}};
    renderer.drawPixelsOverlay(pixels, 5.0f);
    renderer.drawPixelsWCamera(pixels, camera, 5.0f);

    // draw grid
    renderer.drawGrid(camera, 10.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    // Create a small PNG 1x1 pixel (white) in a temp file.
    const std::string tmpPath = (std::filesystem::temp_directory_path() / "test_tex.png").string();
    unsigned char texPixels[3] = {255, 255, 255};
    stbi_write_png(tmpPath.c_str(), 1, 1, 3, texPixels, 1 * 3);

    GLuint texId = renderer.loadTexture(tmpPath);
    EXPECT_NE(texId, 0u);

    graphics::Sprite2D sprite;
    sprite.position = glm::vec2(0.0f);
    sprite.size = glm::vec2(1.0f);
    sprite.textureID = texId;

    renderer.drawSprite(sprite, camera);
    renderer.unloadTexture(texId);

    std::filesystem::remove(tmpPath);

    testCommon.ShutdownImGuiForTests(window, glContext);
}
