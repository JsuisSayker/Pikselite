#include <gtest/gtest.h>

#include <graphics/imgui/components/bars.hpp>
#include <tests/imgui_setup_for_test.hpp>

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

TEST(BarDrawTests, HorizontalOrientationWithoutOffset) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    imguiTest::ImGuiTestCommon testCommon;
    testCommon.InitImGuiForTests(window, glContext);
    // InitImGuiForTests(window, glContext);

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

    testCommon.ShutdownImGuiForTests(window, glContext);
}

TEST(BarDrawTests, InvisibleDoesNotRunContent) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    imguiTest::ImGuiTestCommon testCommon;
    testCommon.InitImGuiForTests(window, glContext);

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

    testCommon.ShutdownImGuiForTests(window, glContext);
}

TEST(BarDrawTests, VisibleRunsContentAndSupportsOrientation) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    imguiTest::ImGuiTestCommon testCommon;
    testCommon.InitImGuiForTests(window, glContext);

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

    testCommon.ShutdownImGuiForTests(window, glContext);
}
