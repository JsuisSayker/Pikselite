#include <engine/pixels/simulation/simulation.hpp>
#include <gtest/gtest.h>
#include <tests/testHelpers.hpp>

TEST(SimulationLoadTests, MediumGridMixedElements)
{
    ChunkGrid grid;
    Simulation sim(grid);

    fillRect(grid, 0, 0, 50, 1, Element::STONE);
    fillRect(grid, 0, 1, 50, 49, Element::SAND);

    TimedScope ts("MediumGridMixedElements", 10.0);
    for (int i = 0; i < 60; ++i)
        sim.update();
}

TEST(SimulationLoadTests, LargeSandFallChain)
{
    ChunkGrid grid;
    Simulation sim(grid);

    fillRect(grid, 0, 0, 1, 1, Element::STONE);
    for (int y = 1; y <= 30; ++y)
        placePixelG(grid, 0, y, Element::SAND);

    TimedScope ts("LargeSandFallChain", 10.0);
    for (int i = 0; i < 60; ++i)
        sim.update();
}

TEST(SimulationLoadTests, FireSpreadAlongCombustible)
{
    ChunkGrid grid;
    Simulation sim(grid);

    for (int x = 0; x < 20; ++x)
    {
        Element::Pixel burning;
        burning.type = Element::WOOD;
        burning.isBurning = true;
        burning.burnTimer = g_elements[Element::WOOD].fireParams.burnDuration;
        grid.setPixel(x, 0, burning);
    }

    TimedScope ts("FireSpreadAlongCombustible", 10.0);
    for (int i = 0; i < 50; ++i)
        sim.update();

    bool allBurnt = true;
    for (int x = 0; x < 20; ++x)
    {
        auto pixel = grid.getPixel(x, 0);
        if (pixel.type == Element::WOOD)
        {
            allBurnt = false;
            break;
        }
    }
    EXPECT_TRUE(allBurnt) << "Not all wood burned after 50 ticks";
}

TEST(SimulationLoadTests, CoordinateExtremes)
{
    ChunkGrid grid;
    Simulation sim(grid);

    placePixelG(grid, 10000, 10000, Element::SAND);
    placePixelG(grid, -10000, 10000, Element::WATER);
    placePixelG(grid, 10000, -10000, Element::FIRE);
    placePixelG(grid, -10000, -10000, Element::STONE);

    for (int i = 0; i < 10; ++i)
        sim.update();

    EXPECT_EQ(grid.getPixel(10000, 10000).type, Element::EMPTY);
    EXPECT_EQ(grid.getPixel(-10000, 10000).type, Element::EMPTY);
    EXPECT_EQ(grid.getPixel(10000, -10000).type, Element::EMPTY);
    EXPECT_EQ(grid.getPixel(-10000, -10000).type, Element::STONE);
}

TEST(SimulationLoadTests, ManyChunks)
{
    ChunkGrid grid;
    Simulation sim(grid);

    for (int cx = -5; cx < 5; ++cx)
        for (int cy = -5; cy < 5; ++cy)
            placePixelG(grid, cx * 32, cy * 32, Element::STONE);

    TimedScope ts("ManyChunks", 10.0);
    for (int i = 0; i < 30; ++i)
        sim.update();
}

TEST(SimulationLoadTests, ParticleUpdateStress)
{
    ChunkGrid grid;
    Simulation sim(grid);

    for (int i = 0; i < 100; ++i)
        sim.spawnParticle(Element::FIRE, {static_cast<float>(i), 100.0f}, {0.0f, -50.0f}, 0, 60);

    TimedScope ts("ParticleUpdateStress", 10.0);
    for (int i = 0; i < 30; ++i)
        sim.update();

    EXPECT_LE(sim.getParticles().size(), 100u);
}

TEST(SimulationLoadTests, VerticalSandColumn)
{
    ChunkGrid grid;
    Simulation sim(grid);

    for (int y = 0; y < 50; ++y)
        placePixelG(grid, 0, y, Element::SAND);

    TimedScope ts("VerticalSandColumn", 10.0);
    for (int i = 0; i < 60; ++i)
        sim.update();
}

TEST(SimulationLoadTests, HorizontalWaterLayer)
{
    ChunkGrid grid;
    Simulation sim(grid);

    fillRect(grid, 0, 0, 1, 1, Element::STONE);
    for (int x = 0; x < 60; ++x)
        placePixelG(grid, x, 1, Element::WATER);

    TimedScope ts("HorizontalWaterLayer", 10.0);
    for (int i = 0; i < 40; ++i)
        sim.update();
}

TEST(SimulationLoadTests, RapidGridReplacement)
{
    ChunkGrid grid;
    Simulation sim(grid);

    TimedScope ts("RapidGridReplacement", 10.0);
    for (int i = 0; i < 20; ++i)
    {
        ChunkGrid newGrid;
        fillRect(newGrid, 0, 0, 10, 10, Element::SAND);
        sim.setGrid(newGrid);

        for (int j = 0; j < 5; ++j)
            sim.update();
    }
}
