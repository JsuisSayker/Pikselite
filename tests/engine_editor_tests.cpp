#include <gtest/gtest.h>
#include <filesystem>

#include <SDL2/SDL.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

#include <engine/pixels/chunk.hpp>
#include <engine/pixels/simulation/simulation.hpp>
#include <editors/project/projectEditor.hpp>
#include <editors/sprite/spriteEditor.hpp>
#include <engine/pixels/pixelEnum.hpp>
#include <graphics/imgui/components/bars.hpp>
#include <graphics/renderer/renderer.hpp>
#include <graphics/renderer/camera.hpp>
#include <GL/glew.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace Pixel;

static void InitImGuiForTests(SDL_Window*& window, SDL_GLContext& glContext) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              128, 128, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    glContext = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    glewInit();

    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");
}

static void ShutdownImGuiForTests(SDL_Window* window, SDL_GLContext glContext) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void writeSimpleSpriteData(const std::string& filename) {
    std::ofstream fout(filename, std::ios::binary);
    ASSERT_TRUE(fout.good());

    // 1 chunk
    uint32_t numChunks = 1;
    fout.write(reinterpret_cast<const char*>(&numChunks), sizeof(numChunks));

    int32_t cx = 0, cy = 0;
    fout.write(reinterpret_cast<const char*>(&cx), sizeof(cx));
    fout.write(reinterpret_cast<const char*>(&cy), sizeof(cy));

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            PixelEntityID id = (x == 0 && y == 0) ? 1 : EMPTY;
            fout.write(reinterpret_cast<const char*>(&id), sizeof(id));
        }
    }

    // oldRenderIndex mapping
    uint32_t count = 1;
    fout.write(reinterpret_cast<const char*>(&count), sizeof(count));
    PixelEntityID id = 1;
    int index = 0;
    fout.write(reinterpret_cast<const char*>(&id), sizeof(id));
    fout.write(reinterpret_cast<const char*>(&index), sizeof(index));

    // solids
    count = 1;
    fout.write(reinterpret_cast<const char*>(&count), sizeof(count));
    fout.write(reinterpret_cast<const char*>(&id), sizeof(id));

    // liquids
    count = 0;
    fout.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // gases
    count = 0;
    fout.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // pixels
    uint32_t numPixels = 1;
    fout.write(reinterpret_cast<const char*>(&numPixels), sizeof(numPixels));
    float px = 0.0f, py = 0.0f, r = 0.1f, g = 0.2f, b = 0.3f;
    fout.write(reinterpret_cast<const char*>(&px), sizeof(px));
    fout.write(reinterpret_cast<const char*>(&py), sizeof(py));
    fout.write(reinterpret_cast<const char*>(&r), sizeof(r));
    fout.write(reinterpret_cast<const char*>(&g), sizeof(g));
    fout.write(reinterpret_cast<const char*>(&b), sizeof(b));

    fout.close();
}

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

TEST(ProjectEditorTests, LoadPlacementAndPlacePendingSprite) {
    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "project_editor_test_sprite.dat";
    writeSimpleSpriteData(tmp.string());

    editors::ProjectEditor editor(nullptr, nullptr, nullptr);
    EXPECT_TRUE(editor.testLoadSpriteForPlacement(tmp.string()));
    editor.testPlacePendingSpriteAtGrid(0, 0);

    auto pixels = editor.getPixels();
    auto gameObjects = editor.getGameObjects();
    auto attrs = editor.getPixelAttributes();
    EXPECT_EQ(pixels.size(), 1);
    EXPECT_EQ(gameObjects.size(), 1);
    EXPECT_EQ(attrs.renderIndex.size(), 1);

    EXPECT_EQ(editor.getChunkGrid().getPixel(0, 0), 1);

    std::filesystem::remove(tmp);
}

TEST(SpriteEditorTests, AddRemoveSaveLoadRoundtrip) {
    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "sprite_editor_test.dat";

    editors::SpriteEditor editor(nullptr, nullptr, nullptr);
    editors::SpriteEditor loaded(nullptr, nullptr, nullptr);

    Pixel::DefaultPixelProperties props;
    props.color = glm::vec3(0.1f, 0.2f, 0.3f);
    props.isSolid = true;

    editor.testAddPixel({0.0f, 0.0f}, props);
    EXPECT_NE(editor.testGetPixelAt({0.0f, 0.0f}), nullptr);

    EXPECT_TRUE(editor.testSaveSpriteToFile(tmp.string()));

    EXPECT_TRUE(loaded.testLoadSpriteFromFile(tmp.string()));
    EXPECT_EQ(loaded.testGetPixelAt({0.0f, 0.0f})->position, glm::vec2(0.0f, 0.0f));

    EXPECT_TRUE(editor.testRemovePixelAt({0.0f, 0.0f}));
    EXPECT_EQ(editor.testGetPixelAt({0.0f, 0.0f}), nullptr);

    std::filesystem::remove(tmp);
}

TEST(SpriteEditorTests, RunHandlesMouseAndKeyboardAndImgui) {
    // Setup SDL/GL + ImGui via the project's graphics interface
    graphics::Interface iface(128, 128);
    graphics::Renderer renderer(iface.getWindow(), iface.getGLContext());
    graphics::ImguiInterface imgui(iface.getWindow(), iface.getGLContext());

    editors::SpriteEditor editor(&iface, &renderer, &imgui);

    // Ensure running with no event does not crash
    editor.run({graphics::NO_EVENT, 0});

    // Test mouse click adds a pixel
    SDL_WarpMouseInWindow(iface.getWindow(), 10, 10);
    editor.run({graphics::MOUSE_LEFT_CLICK, iface.getWindowID()});
    EXPECT_NE(editor.testGetPixelAt(editor.testScreenToWorld({10, 10})), nullptr);

    // Test eraser: toggle eraser and click again
    editor.testSetEraserActive(true);
    editor.run({graphics::MOUSE_LEFT_CLICK, iface.getWindowID()});
    EXPECT_EQ(editor.testGetPixelAt(editor.testScreenToWorld({10, 10})), nullptr);

    // Test mouse drag removes and re-adds pixels
    SDL_WarpMouseInWindow(iface.getWindow(), 0, 0);
    editor.testSetEraserActive(true);
    editor.run({graphics::MOUSE_LEFT_DRAG, iface.getWindowID()});
    EXPECT_EQ(editor.testGetPixelAt(editor.testScreenToWorld({0, 0})), nullptr);

    editor.testSetEraserActive(false);
    editor.run({graphics::MOUSE_LEFT_DRAG, iface.getWindowID()});
    EXPECT_NE(editor.testGetPixelAt(editor.testScreenToWorld({0, 0})), nullptr);

    // Test keyboard movement affects camera (no crash)
    editor.run({graphics::KEY_W, iface.getWindowID()});
    editor.run({graphics::KEY_S, iface.getWindowID()});
    editor.run({graphics::KEY_A, iface.getWindowID()});
    editor.run({graphics::KEY_D, iface.getWindowID()});
    editor.run({graphics::KEY_I, iface.getWindowID()});
    editor.run({graphics::KEY_O, iface.getWindowID()});

    // Test imgui branches (default properties & pixel editor)
    editor.testAddPixel({0.0f, 0.0f}, Pixel::DefaultPixelProperties());
    editor.testSetCurrentPixelIndex(0);
    editor.testSetShowPixelEditor(true);
    editor.run({graphics::NO_EVENT, iface.getWindowID()});

    editor.testSetShowDefaultPropertiesEditor(true);
    editor.run({graphics::NO_EVENT, iface.getWindowID()});

    // Test sprite placement flow - create a temporary sprite file
    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "sprite_editor_test_place.dat";
    writeSimpleSpriteData(tmp.string());

    EXPECT_TRUE(editor.testLoadSpriteForPlacement(tmp.string()));
    editor.testPlacePendingSpriteAtWorld({0.0f, 0.0f});

    std::filesystem::remove(tmp);
}

TEST(SpriteEditorTests, KeyboardShortcutsLoadSaveViaImgui) {
    graphics::Interface iface(128, 128);
    graphics::Renderer renderer(iface.getWindow(), iface.getGLContext());
    graphics::ImguiInterface imgui(iface.getWindow(), iface.getGLContext());

    editors::SpriteEditor editor(&iface, &renderer, &imgui);

    // Exercise keyboard shortcuts to alter camera (no crash)
    editor.run({graphics::KEY_I, iface.getWindowID()});
    editor.run({graphics::KEY_O, iface.getWindowID()});

    // Setup a temporary sprite file and load it through the imgui flow
    std::filesystem::path tmpLoad = std::filesystem::temp_directory_path() / "sprite_editor_test_load.dat";
    writeSimpleSpriteData(tmpLoad.string());
    editor.testSetCurrentSpriteFilename(tmpLoad.string());
    editor.run({graphics::NO_EVENT, iface.getWindowID()});

    // Place the loaded sprite and verify it appears
    editor.testPlacePendingSpriteAtWorld({0.0f, 0.0f});
    EXPECT_NE(editor.testGetPixelAt({0.0f, 0.0f}), nullptr);

    // Save via the imgui flow
    std::filesystem::path tmpSave = std::filesystem::temp_directory_path() / "sprite_editor_test_save.dat";
    editor.testSetNewSpritePath(tmpSave.string());
    editor.run({graphics::NO_EVENT, iface.getWindowID()});
    EXPECT_TRUE(std::filesystem::exists(tmpSave));

    std::filesystem::remove(tmpLoad);
    std::filesystem::remove(tmpSave);
}

TEST(BarConfigTests, Initialization) {
    graphics::BarConfig config;
    EXPECT_EQ(config.orientation, graphics::BarOrientation::Horizontal);
    EXPECT_EQ(config.label, "");
    EXPECT_TRUE(config.size.x == 0 && config.size.y == 0);
    EXPECT_TRUE(config.visible);
    EXPECT_TRUE(config.position.x == 0 && config.position.y == 0);
}

TEST(BarTests, IsVisible) {
    graphics::BarConfig config;
    config.visible = true;
    graphics::Bar bar(config);
    EXPECT_TRUE(bar.IsVisible());

    config.visible = false;
    graphics::Bar bar2(config);
    EXPECT_FALSE(bar2.IsVisible());
}

TEST(GetDesiredPositionTests, Top) {
    ImVec2 pos = graphics::GetDesiredPosition("top");
    EXPECT_TRUE(pos.x == 0 && pos.y == 0);
}

TEST(GetDesiredPositionTests, Bottom) {
    ImVec2 pos = graphics::GetDesiredPosition("bottom");
    EXPECT_TRUE(pos.x == 0 && pos.y == -1);
}

TEST(GetDesiredPositionTests, Left) {
    ImVec2 pos = graphics::GetDesiredPosition("left");
    EXPECT_TRUE(pos.x == 0 && pos.y == 0);
}

TEST(GetDesiredPositionTests, Right) {
    ImVec2 pos = graphics::GetDesiredPosition("right");
    EXPECT_TRUE(pos.x == -1 && pos.y == 0);
}

TEST(GetDesiredPositionTests, Default) {
    ImVec2 pos = graphics::GetDesiredPosition("unknown");
    EXPECT_TRUE(pos.x == 0 && pos.y == 0);
}

TEST(GetDesiredSizeTests, NotFull) {
    EXPECT_EQ(graphics::GetDesiredSize("notfull", graphics::BarOrientation::Horizontal), 0);
    EXPECT_EQ(graphics::GetDesiredSize("notfull", graphics::BarOrientation::Vertical), 0);
}

TEST(GetDesiredSizeTests, FullReturnsDisplaySize) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    InitImGuiForTests(window, glContext);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(640, 480);

    EXPECT_EQ(graphics::GetDesiredSize("full", graphics::BarOrientation::Horizontal), 640);
    EXPECT_EQ(graphics::GetDesiredSize("full", graphics::BarOrientation::Vertical), 480);

    ShutdownImGuiForTests(window, glContext);
}

TEST(BarDrawTests, HorizontalOrientationWithoutOffset) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    InitImGuiForTests(window, glContext);

    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    graphics::BarConfig cfg;
    cfg.visible = true;
    cfg.label = "test";
    cfg.size = ImVec2(200, 20);
    cfg.position = ImVec2(10, 10); // no negative offset
    cfg.orientation = graphics::BarOrientation::Horizontal;

    graphics::Bar bar(cfg);
    bool called = false;
    bar.Draw([&] { called = true; });
    EXPECT_TRUE(called);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ShutdownImGuiForTests(window, glContext);
}

TEST(RendererTests, BasicDrawPaths) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    InitImGuiForTests(window, glContext);

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

    ShutdownImGuiForTests(window, glContext);
}

TEST(BarDrawTests, InvisibleDoesNotRunContent) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    InitImGuiForTests(window, glContext);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800, 600);

    graphics::BarConfig cfg;
    cfg.visible = false;
    cfg.label = "test";
    cfg.size = ImVec2(100, 10);
    cfg.position = ImVec2(-1, -1);

    graphics::Bar bar(cfg);
    bool called = false;
    bar.Draw([&] { called = true; });
    EXPECT_FALSE(called);

    ShutdownImGuiForTests(window, glContext);
}

