#include <graphics/imgui/components/bars.hpp>
#include <gtest/gtest.h>
#include <tests/imguiSetupForTest.hpp>

TEST(GetDesiredSizeTests, NotFull)
{
    EXPECT_EQ(graphics::GetDesiredSize("notfull", graphics::BarOrientation::Horizontal), 0);
    EXPECT_EQ(graphics::GetDesiredSize("notfull", graphics::BarOrientation::Vertical), 0);
}

TEST(GetDesiredSizeTests, FullReturnsDisplaySize)
{
    SDL_Window*                window    = nullptr;
    SDL_GLContext              glContext = nullptr;
    imguiTest::ImGuiTestCommon testCommon;
    testCommon.InitImGuiForTests(window, glContext);

    ImGuiIO& io    = ImGui::GetIO();
    io.DisplaySize = ImVec2(640, 480);

    EXPECT_EQ(graphics::GetDesiredSize("full", graphics::BarOrientation::Horizontal), 0);
    EXPECT_EQ(graphics::GetDesiredSize("full", graphics::BarOrientation::Vertical), 0);

    testCommon.ShutdownImGuiForTests(window, glContext);
}
