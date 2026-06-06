#include <filesystem>
#include <graphics/renderer/renderer.hpp>
#include <gtest/gtest.h>
#include <tests/imguiSetupForTest.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

TEST(RendererTests, BasicDrawPaths)
{
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
