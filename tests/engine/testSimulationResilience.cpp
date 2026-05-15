#include <engine/pixels/simulation/chunk.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <engine/pixels/simulation/simulation.hpp>
#include <gtest/gtest.h>

//
// Coordinate system: GRAVITY_DIR = -1, so "down" = decreasing y.
// y=0 is the bottom of the simulation space.
//

namespace
{
    void placePixel(ChunkGrid& grid, int x, int y, Element::ElementType type)
    {
        Element::Pixel p;
        p.type = type;
        grid.setPixel(x, y, p);
    }
} // namespace

TEST(SimulationResilienceTests, AllEmptyGrid)
{
    ChunkGrid grid;
    Simulation sim(grid);

    for (int i = 0; i < 10; ++i)
        sim.update();
}

TEST(SimulationResilienceTests, SingleSandFallsDown)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Sand at y=1 falls to y=0 (bottom)
    placePixel(grid, 0, 1, Element::SAND);
    sim.update();

    EXPECT_EQ(grid.getPixel(0, 1).type, Element::EMPTY);
    EXPECT_EQ(grid.getPixel(0, 0).type, Element::SAND);
}

TEST(SimulationResilienceTests, SingleWaterFallsDown)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Water at y=3 falls to y=2, then eventually to y=0
    placePixel(grid, 3, 3, Element::WATER);
    sim.update();

    EXPECT_EQ(grid.getPixel(3, 3).type, Element::EMPTY);
    EXPECT_EQ(grid.getPixel(3, 2).type, Element::WATER);
}

TEST(SimulationResilienceTests, WaterDispersesOnSurface)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Continuous stone floor at y=0 to prevent water from falling through
    for (int x = 0; x < 20; ++x)
        placePixel(grid, x, 0, Element::STONE);

    // Water on top of the stone floor at y=1
    placePixel(grid, 10, 1, Element::WATER);

    // Water should spread horizontally along y=1 (can't fall through stone)
    for (int i = 0; i < 30; ++i)
        sim.update();

    // Stone floor remains
    EXPECT_EQ(grid.getPixel(10, 0).type, Element::STONE);

    // Water should have spread left or right from origin
    bool foundWater = false;
    for (int x = 0; x < 20; ++x)
    {
        if (grid.getPixel(x, 1).type == Element::WATER)
        {
            foundWater = true;
            break;
        }
    }
    EXPECT_TRUE(foundWater);
}

TEST(SimulationResilienceTests, FireBurnsOut)
{
    ChunkGrid grid;
    Simulation sim(grid);

    placePixel(grid, 0, 0, Element::FIRE);

    for (int i = 0; i < 20; ++i)
        sim.update();

    EXPECT_EQ(grid.getPixel(0, 0).type, Element::EMPTY);
}

TEST(SimulationResilienceTests, StoneIsStatic)
{
    ChunkGrid grid;
    Simulation sim(grid);

    placePixel(grid, 10, 0, Element::STONE);

    for (int i = 0; i < 10; ++i)
        sim.update();

    EXPECT_EQ(grid.getPixel(10, 0).type, Element::STONE);
}

TEST(SimulationResilienceTests, SandBlockedByStone)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Stone floor
    placePixel(grid, 5, 0, Element::STONE);
    // Sand on top
    placePixel(grid, 5, 1, Element::SAND);

    sim.update();

    // Sand tries to fall to y=0 but is blocked by stone.
    // It should have moved diagonally (left-down or right-down).
    bool diagRight = grid.getPixel(6, 0).type == Element::SAND;
    bool diagLeft = grid.getPixel(4, 0).type == Element::SAND;
    EXPECT_TRUE(diagRight || diagLeft);
}

TEST(SimulationResilienceTests, ChunkBoundaryCrossing)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Place a pixel at the very bottom of the topmost chunk row (y=0).
    // With GRAVITY_DIR=-1, it falls to y=-1 (next chunk).
    placePixel(grid, 0, 0, Element::SAND);
    sim.update();

    EXPECT_EQ(grid.getPixel(0, 0).type, Element::EMPTY);
    EXPECT_EQ(grid.getPixel(0, -1).type, Element::SAND);
}

TEST(SimulationResilienceTests, WaterLavaCreatesStone)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Build a stone container around (0,0) to prevent lava from moving
    // Lava tries to fall and spread; stone walls keep it in place
    placePixel(grid, -1, -1, Element::STONE);
    placePixel(grid, -1, 0, Element::STONE);
    placePixel(grid, 0, -1, Element::STONE);
    placePixel(grid, 1, -1, Element::STONE);
    placePixel(grid, 1, 0, Element::STONE);

    // Lava at (0,0) trapped by stone
    placePixel(grid, 0, 0, Element::LAVA);
    // Water above lava (check range is 3 in updateWater)
    placePixel(grid, 0, 3, Element::WATER);

    sim.update();

    // Water should find lava 3 cells below and convert it
    // Lava -> STONE, Water -> FIRE
    EXPECT_EQ(grid.getPixel(0, 0).type, Element::STONE);
    EXPECT_EQ(grid.getPixel(0, 3).type, Element::FIRE);
}

TEST(SimulationResilienceTests, SandFallsThroughWater)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Sand (density 5) above water (density 2) — sand sinks through water
    // Place water at y=0 and sand at y=1
    placePixel(grid, 0, 0, Element::WATER);
    placePixel(grid, 0, 1, Element::SAND);

    sim.update();

    // Sand should have moved down into water (swap or replace)
    // After one tick: either sand at y=0 (swapped) or sand at y=-1 (fell through)
    EXPECT_TRUE(grid.getPixel(0, 0).type == Element::SAND ||
                grid.getPixel(0, -1).type == Element::SAND);
}

TEST(SimulationResilienceTests, MultipleElementsCoexist)
{
    ChunkGrid grid;
    Simulation sim(grid);

    // Mix of elements at different positions, all at y=0
    placePixel(grid, 0, 0, Element::STONE);

    // Spread other elements where they can fall without interference
    placePixel(grid, 3, 3, Element::SAND);
    placePixel(grid, 6, 6, Element::WATER);
    placePixel(grid, 9, 0, Element::FIRE);

    for (int i = 0; i < 15; ++i)
        sim.update();

    // Stone should remain
    EXPECT_EQ(grid.getPixel(0, 0).type, Element::STONE);

    // Fire should have burned out
    EXPECT_EQ(grid.getPixel(9, 0).type, Element::EMPTY);

    // Sand and water should have moved from their starting positions
    EXPECT_EQ(grid.getPixel(3, 3).type, Element::EMPTY);
    EXPECT_EQ(grid.getPixel(6, 6).type, Element::EMPTY);
}
