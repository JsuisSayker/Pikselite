#include <gtest/gtest.h>
#include <engine/pixels/chunk.hpp>

using namespace Pixel;

TEST(ChunkTests, SetGetBoundary) {
    Chunk c;
    EXPECT_EQ(c.get(0, 0), EMPTY);
    EXPECT_EQ(c.get(CHUNKS_SIZE, 0), EMPTY);

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
