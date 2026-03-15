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

using namespace Pixel;

using namespace Pixel;

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

static void InitImGuiForTests(SDL_Window*& window, SDL_GLContext& glContext) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              128, 128, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    glContext = SDL_GL_CreateContext(window);

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

TEST(BarDrawTests, VisibleRunsContentAndSupportsOrientation) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    InitImGuiForTests(window, glContext);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800, 600);

    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    graphics::BarConfig cfg;
    cfg.visible = true;
    cfg.label = "test";
    cfg.size = ImVec2(200, 20);
    cfg.position = ImVec2(-1, -1);
    cfg.orientation = graphics::BarOrientation::Vertical;

    graphics::Bar bar(cfg);
    bool called = false;
    bar.Draw([&] { called = true; });
    EXPECT_TRUE(called);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ShutdownImGuiForTests(window, glContext);
}
