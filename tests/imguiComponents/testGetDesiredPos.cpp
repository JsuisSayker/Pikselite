#include <graphics/imgui/components/bars.hpp>
#include <gtest/gtest.h>

TEST(GetDesiredPositionTests, Top)
{
    ImVec2 pos = graphics::GetDesiredPosition("top");
    EXPECT_TRUE(pos.x == 0 && pos.y == 0);
}

TEST(GetDesiredPositionTests, Bottom)
{
    ImVec2 pos = graphics::GetDesiredPosition("bottom");
    EXPECT_TRUE(pos.x == 0 && pos.y == -1);
}

TEST(GetDesiredPositionTests, Left)
{
    ImVec2 pos = graphics::GetDesiredPosition("left");
    EXPECT_TRUE(pos.x == 0 && pos.y == 0);
}

TEST(GetDesiredPositionTests, Right)
{
    ImVec2 pos = graphics::GetDesiredPosition("right");
    EXPECT_TRUE(pos.x == -1 && pos.y == 0);
}

TEST(GetDesiredPositionTests, Default)
{
    ImVec2 pos = graphics::GetDesiredPosition("unknown");
    EXPECT_TRUE(pos.x == 0 && pos.y == 0);
}