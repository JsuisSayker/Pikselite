#include <gtest/gtest.h>
#include <filesystem>

#include <editors/project/ProjectEditor.hpp>
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
