#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include <editors/project/projectEditor.hpp>
#include <engine/pixels/chunk.hpp>
#include <engine/pixels/pixelEnum.hpp>

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

TEST(ProjectEditorTests, LoadPlacementAndPlacePendingSprite) {
    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "project_editor_test_sprite.dat";
    writeSimpleSpriteData(tmp.string());

    editors::ProjectEditor editor(nullptr, nullptr, nullptr, nullptr);
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

TEST(ProjectEditorTests, SetSceneDataReplacesEditorState) {
    editors::ProjectEditor editor(nullptr, nullptr, nullptr, nullptr);

    std::vector<graphics::Pixel> renderPixels{
        {{20.0f, 30.0f}, {0.9f, 0.1f, 0.2f}}
    };

    Pixel::GameObject object;
    object.id = 12;
    object.name = "LoadedObject";
    object.pixelEntities = {77};
    std::vector<Pixel::GameObject> gameObjects{object};

    Pixel::PixelAttributes attributes;
    attributes.renderIndex[77] = 0;
    attributes.solidAttributes[77] = Pixel::Solid{};

    Pixel::ChunkGrid grid;
    grid.getOrCreateChunk(0, 0).set(2, 3, 77);

    editor.setSceneData(renderPixels, gameObjects, attributes, grid, 42, 9);

    const auto loadedPixels = editor.getPixels();
    const auto loadedObjects = editor.getGameObjects();
    const auto loadedAttributes = editor.getPixelAttributes();
    auto loadedGrid = editor.getChunkGrid();

    ASSERT_EQ(loadedPixels.size(), 1u);
    EXPECT_FLOAT_EQ(loadedPixels[0].position.x, 20.0f);
    EXPECT_FLOAT_EQ(loadedPixels[0].position.y, 30.0f);
    EXPECT_FLOAT_EQ(loadedPixels[0].color.r, 0.9f);
    EXPECT_FLOAT_EQ(loadedPixels[0].color.g, 0.1f);
    EXPECT_FLOAT_EQ(loadedPixels[0].color.b, 0.2f);

    ASSERT_EQ(loadedObjects.size(), 1u);
    EXPECT_EQ(loadedObjects[0].id, 12u);
    EXPECT_EQ(loadedObjects[0].name, "LoadedObject");
    ASSERT_EQ(loadedObjects[0].pixelEntities.size(), 1u);
    EXPECT_EQ(loadedObjects[0].pixelEntities[0], 77u);

    EXPECT_EQ(loadedGrid.getPixel(2, 3), 77u);
    EXPECT_EQ(loadedAttributes.renderIndex.count(77), 1u);
    EXPECT_EQ(loadedAttributes.solidAttributes.count(77), 1u);

    EXPECT_EQ(editor.getPixelIdCounter(), 42u);
    EXPECT_EQ(editor.getGameObjectCounter(), 9u);
}

TEST(ProjectEditorTests, SetSceneDataCountersAreUsedWhenPlacingSprite) {
    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "project_editor_test_sprite_counters.dat";
    writeSimpleSpriteData(tmp.string());

    editors::ProjectEditor editor(nullptr, nullptr, nullptr, nullptr);
    editor.setSceneData({}, {}, Pixel::PixelAttributes{}, Pixel::ChunkGrid{}, 100, 50);

    ASSERT_TRUE(editor.testLoadSpriteForPlacement(tmp.string()));
    editor.testPlacePendingSpriteAtGrid(0, 0);

    const auto objects = editor.getGameObjects();
    const auto attributes = editor.getPixelAttributes();

    ASSERT_EQ(objects.size(), 1u);
    EXPECT_EQ(objects[0].id, 50u);
    ASSERT_EQ(objects[0].pixelEntities.size(), 1u);
    EXPECT_EQ(objects[0].pixelEntities[0], 100u);
    EXPECT_EQ(editor.getChunkGrid().getPixel(0, 0), 100u);
    EXPECT_EQ(attributes.renderIndex.count(100u), 1u);
    EXPECT_EQ(editor.getPixelIdCounter(), 101u);
    EXPECT_EQ(editor.getGameObjectCounter(), 51u);

    std::filesystem::remove(tmp);
}

TEST(ProjectEditorTests, RunExecutesAndPreservesSceneWithoutInput) {
    graphics::Interface iface(128, 128);
    graphics::Renderer renderer(iface.getWindow(), iface.getGLContext());
    graphics::ImguiInterface imgui(iface.getWindow(), iface.getGLContext());

    editors::ProjectEditor editor(&iface, &renderer, &imgui, nullptr);

    std::vector<graphics::Pixel> renderPixels{
        {{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}
    };
    Pixel::PixelAttributes attributes;
    attributes.renderIndex[5] = 0;
    Pixel::ChunkGrid grid;
    grid.getOrCreateChunk(0, 0).set(0, 0, 5);

    editor.setSceneData(renderPixels, {}, attributes, grid, 6, 1);

    editor.run({graphics::NO_EVENT, iface.getWindowID()});
    editor.run({graphics::KEY_W, iface.getWindowID()});
    editor.run({graphics::KEY_A, iface.getWindowID()});
    editor.run({graphics::KEY_S, iface.getWindowID()});
    editor.run({graphics::KEY_D, iface.getWindowID()});
    editor.run({graphics::KEY_I, iface.getWindowID()});
    editor.run({graphics::KEY_O, iface.getWindowID()});
    editor.run({graphics::KEY_L, iface.getWindowID()}); // exercises KEY_L branch in handleEvents

    const auto pixelsAfterRun = editor.getPixels();
    ASSERT_EQ(pixelsAfterRun.size(), 1u);
    EXPECT_EQ(editor.getChunkGrid().getPixel(0, 0), 5u);
    EXPECT_FALSE(editor.consumeSaveSceneRequest());
    EXPECT_FALSE(editor.consumeLoadSceneRequest());
}
