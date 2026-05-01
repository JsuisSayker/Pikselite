#include <editors/sprite/spriteEditor.hpp>
#include <engine/pixels/chunk.hpp>
#include <engine/pixels/pixelEnum.hpp>
#include <filesystem>
#include <gtest/gtest.h>

using namespace Pixel;

static void writeSimpleSpriteData(const std::string& filename)
{
    std::ofstream fout(filename, std::ios::binary);
    ASSERT_TRUE(fout.good());

    // 1 chunk
    uint32_t numChunks = 1;
    fout.write(reinterpret_cast<const char*>(&numChunks), sizeof(numChunks));

    int32_t cx = 0, cy = 0;
    fout.write(reinterpret_cast<const char*>(&cx), sizeof(cx));
    fout.write(reinterpret_cast<const char*>(&cy), sizeof(cy));

    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
        for (int y = 0; y < CHUNK_SIZE; ++y)
        {
            PixelEntityID id = (x == 0 && y == 0) ? 1 : EMPTY;
            fout.write(reinterpret_cast<const char*>(&id), sizeof(id));
        }
    }

    // oldRenderIndex mapping
    uint32_t count = 1;
    fout.write(reinterpret_cast<const char*>(&count), sizeof(count));
    PixelEntityID id    = 1;
    int           index = 0;
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

TEST(SpriteEditorTests, AddRemoveSaveLoadRoundtrip)
{
    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "sprite_editor_test.dat";

    editors::SpriteEditor editor(nullptr, nullptr, nullptr);
    editors::SpriteEditor loaded(nullptr, nullptr, nullptr);

    Pixel::DefaultPixelProperties props;
    props.color   = glm::vec3(0.1f, 0.2f, 0.3f);
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

TEST(SpriteEditorTests, RunHandlesMouseAndKeyboardAndImgui)
{
    // Setup SDL/GL + ImGui via the project's graphics interface
    graphics::Interface      iface(128, 128);
    graphics::Renderer       renderer(iface.getWindow(), iface.getGLContext());
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
    std::filesystem::path tmp =
        std::filesystem::temp_directory_path() / "sprite_editor_test_place.dat";
    writeSimpleSpriteData(tmp.string());

    EXPECT_TRUE(editor.testLoadSpriteForPlacement(tmp.string()));
    editor.testPlacePendingSpriteAtWorld({0.0f, 0.0f});

    std::filesystem::remove(tmp);
}

TEST(SpriteEditorTests, KeyboardShortcutsLoadSaveViaImgui)
{
    graphics::Interface      iface(128, 128);
    graphics::Renderer       renderer(iface.getWindow(), iface.getGLContext());
    graphics::ImguiInterface imgui(iface.getWindow(), iface.getGLContext());

    editors::SpriteEditor editor(&iface, &renderer, &imgui);

    // Exercise keyboard shortcuts to alter camera (no crash)
    editor.run({graphics::KEY_I, iface.getWindowID()});
    editor.run({graphics::KEY_O, iface.getWindowID()});

    // Setup a temporary sprite file and load it through the imgui flow
    std::filesystem::path tmpLoad =
        std::filesystem::temp_directory_path() / "sprite_editor_test_load.dat";
    writeSimpleSpriteData(tmpLoad.string());
    editor.testSetCurrentSpriteFilename(tmpLoad.string());
    editor.run({graphics::NO_EVENT, iface.getWindowID()});

    // Place the loaded sprite and verify it appears
    editor.testPlacePendingSpriteAtWorld({0.0f, 0.0f});
    EXPECT_NE(editor.testGetPixelAt({0.0f, 0.0f}), nullptr);

    // Save via the imgui flow
    std::filesystem::path tmpSave =
        std::filesystem::temp_directory_path() / "sprite_editor_test_save.dat";
    editor.testSetNewSpritePath(tmpSave.string());
    editor.run({graphics::NO_EVENT, iface.getWindowID()});
    EXPECT_TRUE(std::filesystem::exists(tmpSave));

    std::filesystem::remove(tmpLoad);
    std::filesystem::remove(tmpSave);
}
