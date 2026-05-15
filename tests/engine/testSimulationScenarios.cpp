#include <engine/pixels/simulation/simulation.hpp>
#include <gtest/gtest.h>
#include <tests/testHelpers.hpp>


//
// Coordinate system: GRAVITY_DIR = -1, so "down" = decreasing y.
// y=0 is the bottom of the simulation space.
//

TEST(SimulationScenarioTests, WaterFlowsThroughChannel)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // U-shaped stone channel
    fillRect(grid, 0, 0, 10, 1, Element::STONE);    // floor at y=0
    fillRect(grid, 0, 1, 1, 8, Element::STONE);      // left wall
    fillRect(grid, 9, 1, 1, 8, Element::STONE);      // right wall

    // Water placed at top center
    placePixelG(grid, 5, 8, Element::WATER);

    for (int i = 0; i < 30; ++i)
        sim.update();

    // Water should be at the bottom (y=0 or y=1) inside the channel
    bool waterAtBottom = false;
    for (int x = 1; x < 9; ++x)
    {
        auto pixel = grid.getPixel(x, 1);
        if (pixel.type == Element::WATER)
        {
            waterAtBottom = true;
            break;
        }
    }
    EXPECT_TRUE(waterAtBottom) << "Water did not reach bottom of channel";

    // Water should NOT be outside the walls
    for (int y = 0; y <= 8; ++y)
    {
        EXPECT_NE(grid.getPixel(0, y).type, Element::WATER);
        EXPECT_NE(grid.getPixel(9, y).type, Element::WATER);
    }
}

TEST(SimulationScenarioTests, SandSlidesDownSlope)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Diagonal stone slope from (0,5) to (5,0)
    for (int i = 0; i <= 5; ++i)
        placePixelG(grid, i, 5 - i, Element::STONE);

    // Sand above the highest point of the slope
    placePixelG(grid, 5, 6, Element::SAND);

    for (int i = 0; i < 15; ++i)
        sim.update();

    // Sand should have moved down along the slope
    bool sandOffStart = true;
    auto pixel = grid.getPixel(5, 6);
    if (pixel.type == Element::SAND)
        sandOffStart = false;
    EXPECT_TRUE(sandOffStart) << "Sand did not move from starting position";
}

TEST(SimulationScenarioTests, FirewoodChainReaction)
{
    ChunkGrid grid;
    Simulation sim(grid);

    for (int x = 0; x < 10; ++x)
    {
        Element::Pixel burning;
        burning.type = Element::WOOD;
        burning.isBurning = true;
        burning.burnTimer = g_elements[Element::WOOD].fireParams.burnDuration;
        grid.setPixel(x, 0, burning);
    }

    for (int i = 0; i < 50; ++i)
        sim.update();

    bool allConsumed = true;
    for (int x = 0; x < 10; ++x)
    {
        auto pixel = grid.getPixel(x, 0);
        if (pixel.type == Element::WOOD)
        {
            allConsumed = false;
            break;
        }
    }
    EXPECT_TRUE(allConsumed) << "Fire did not consume all wood in chain reaction";
}

TEST(SimulationScenarioTests, SandSinksThroughWater)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Stone container
    fillRect(grid, 0, 0, 6, 1, Element::STONE);       // floor
    fillRect(grid, 0, 1, 1, 10, Element::STONE);       // left wall
    fillRect(grid, 5, 1, 1, 10, Element::STONE);       // right wall

    // Water filling bottom half
    fillRect(grid, 1, 1, 4, 5, Element::WATER);

    // Sand filling top half (above water)
    fillRect(grid, 1, 6, 4, 5, Element::SAND);

    for (int i = 0; i < 60; ++i)
        sim.update();

    // After sufficient ticks, sand should have sunk below water
    bool sandAtBottom = false;
    bool waterAtTop = false;

    // Check bottom rows for sand
    for (int x = 1; x < 5; ++x)
    {
        if (grid.getPixel(x, 1).type == Element::SAND)
            sandAtBottom = true;
    }

    // Check top rows for water (after sand sinks)
    for (int x = 1; x < 5; ++x)
    {
        if (grid.getPixel(x, 9).type == Element::WATER)
            waterAtTop = true;
    }

    EXPECT_TRUE(sandAtBottom) << "Sand did not sink through water";
    EXPECT_TRUE(waterAtTop) << "Water did not rise above sand";
}

TEST(SimulationScenarioTests, LavaInStonePool)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Stone pool
    fillRect(grid, 0, 0, 6, 1, Element::STONE);      // floor
    fillRect(grid, 0, 1, 1, 5, Element::STONE);       // left wall
    fillRect(grid, 5, 1, 1, 5, Element::STONE);       // right wall

    // Lava in the pool
    placePixelG(grid, 3, 5, Element::LAVA);

    for (int i = 0; i < 20; ++i)
        sim.update();

    // Lava should have fallen to bottom of pool
    bool lavaAtBottom = false;
    for (int x = 1; x < 5; ++x)
    {
        auto pixel = grid.getPixel(x, 1);
        if (pixel.type == Element::LAVA)
        {
            lavaAtBottom = true;
            break;
        }
    }
    EXPECT_TRUE(lavaAtBottom) << "Lava did not fall to bottom of stone pool";
}

TEST(SimulationScenarioTests, RainFillsTerrainPits)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Stone ground with a pit at x=3..7 where y=0 is lowered to y=-2
    fillRect(grid, 0, 0, 3, 1, Element::STONE);       // left ground (x=0..2)
    fillRect(grid, 8, 0, 3, 1, Element::STONE);       // right ground (x=8..10)
    fillRect(grid, 3, -2, 5, 1, Element::STONE);       // pit floor at y=-2 (x=3..7)
    placePixelG(grid, 3, -1, Element::STONE);          // left pit wall
    placePixelG(grid, 7, -1, Element::STONE);          // right pit wall

    // Sand "rain" from above
    placePixelG(grid, 5, 10, Element::SAND);

    for (int i = 0; i < 15; ++i)
        sim.update();

    // Sand should have fallen into the pit (air gap at y=-1, floor at y=-2)
    bool sandInPit = (grid.getPixel(5, -1).type == Element::SAND ||
                      grid.getPixel(5, -2).type == Element::SAND);
    EXPECT_TRUE(sandInPit) << "Sand did not fill the terrain pit";
}

TEST(SimulationScenarioTests, FirebreakStopsSpread)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Left wood line (0..9), firebreak at x=10, right wood line (11..20)
    for (int x = 0; x <= 9; ++x)
        placePixelG(grid, x, 0, Element::WOOD);
    placePixelG(grid, 10, 0, Element::STONE);  // firebreak
    for (int x = 11; x <= 20; ++x)
        placePixelG(grid, x, 0, Element::WOOD);

    for (int x = 0; x <= 9; ++x)
    {
        Element::Pixel burning;
        burning.type = Element::WOOD;
        burning.isBurning = true;
        burning.burnTimer = g_elements[Element::WOOD].fireParams.burnDuration;
        grid.setPixel(x, 0, burning);
    }

    for (int i = 0; i < 50; ++i)
        sim.update();

    EXPECT_EQ(grid.getPixel(10, 0).type, Element::STONE);

    // All left wood should have burned out to EMPTY
    bool allConsumed = true;
    for (int x = 0; x <= 9; ++x)
    {
        auto pixel = grid.getPixel(x, 0);
        if (pixel.type != Element::EMPTY)
        {
            allConsumed = false;
            break;
        }
    }
    EXPECT_TRUE(allConsumed) << "Left wood did not burn out";

    // Right wood should remain intact (protected by firebreak)
    bool rightIntact = true;
    for (int x = 11; x <= 20; ++x)
    {
        if (grid.getPixel(x, 0).type != Element::WOOD)
        {
            rightIntact = false;
            break;
        }
    }
    EXPECT_TRUE(rightIntact) << "Fire crossed the firebreak and damaged right wood";
}

TEST(SimulationScenarioTests, SandColumnCollapses)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Tall column of sand (10 high, 3 wide)
    fillRect(grid, 0, 0, 3, 10, Element::SAND);

    for (int i = 0; i < 30; ++i)
        sim.update();

    // After collapse, top of column should be lower than original
    bool topCollapsed = true;
    for (int y = 9; y > 5; --y)
    {
        for (int x = 0; x < 3; ++x)
        {
            if (grid.getPixel(x, y).type == Element::SAND)
            {
                topCollapsed = false;
                break;
            }
        }
        if (!topCollapsed)
            break;
    }
    EXPECT_TRUE(topCollapsed) << "Sand column did not collapse";
}
