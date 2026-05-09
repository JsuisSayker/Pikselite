#pragma once
#include <algorithm>
#include <box2d/box2d.h>
#include <engine/pixels/simulation/element.hpp>
#include <queue>
#include <unordered_set>
#include <vector>

class Simulation
{
  public:
    /**
     * @brief Binding between one simulated pixel and a body-local position.
     */
    struct BodyPixelBinding
    {
        Element::ElementType type;
        Element::Vec2f       localUV;
        Element::Vec2f       uv;
        int                  gridX;
        int                  gridY;
    };

    /**
     * @brief Pixel bindings and occupancy associated with one Box2D body.
     */
    struct RegionBodyBinding
    {
        b2BodyId                      bodyId;
        std::vector<BodyPixelBinding> pixels;
        Element::ElementType          fillType = Element::STONE;
        std::vector<Element::Vec2i>   occupiedCells;
    };

    /**
     * @brief Creates a simulation over a chunk grid.
     * @param grid Reference to the world grid used by this simulation.
     * @return Constructs `Simulation` and initializes element definitions.
     */
    Simulation(ChunkGrid& grid) : grid(grid)
    {
        initElements();
    }

    /**
     * @brief Initializes all element definitions and update handlers.
     * @return void
     */
    void initElements();

    /**
     * @brief Advances the simulation by one tick.
     * @return void
     */
    void update();

        /**
         * @brief Updates particle behaviors and lifetimes, removing expired particles.
         * @return void
         */
        void updateParticles();

        /**
         * @brief Updates one sand pixel behavior.
         * @param grid Simulation grid.
         * @param x Global X coordinate.
         * @param y Global Y coordinate.
         * @return void
         */
        inline void updateBurning(ChunkGrid& grid, int x, int y);

    /**
     * @brief Resets per-pixel update flags for the current frame.
     * @return void
     */
    void resetUpdatedFlags();

    /**
     * @brief Replaces the current grid and resets region/collider state.
     * @param newGrid Grid value to copy into the simulation.
     * @return void
     */
    void setGrid(ChunkGrid& newGrid);

    /**
     * @brief Returns the mutable simulation grid.
     * @return Reference to the internal `ChunkGrid`.
     */
    ChunkGrid& getGrid()
    {
        return grid;
    }

    /**
     * @brief Builds an ordered chunk list for deterministic updates.
     * @return void
     */
    void orderChunksForUpdate();

    /**
     * @brief Flood-fills one connected region of a given element type.
     * @param x Seed global X coordinate.
     * @param y Seed global Y coordinate.
     * @param type Element type to include in the region.
     * @return Extracted region data.
     */
    Element::Region regionFloodFill(int x, int y, Element::ElementType type);

    /**
     * @brief Computes a packed visited-set key.
     * @param x Global X coordinate.
     * @param y Global Y coordinate.
     * @return Packed 64-bit coordinate key.
     */
    int64_t makeVisitedKey(int x, int y)
    {
        return (static_cast<int64_t>(x) << 32) | (static_cast<uint32_t>(y));
    }

    /**
     * @brief Generates contour edges for a region.
     * @param region Region to mutate.
     * @return void
     */
    void buildRegionContoursMarchingSquare(Element::Region& region);

    /**
     * @brief Simplifies region contours.
     * @param region Region to mutate.
     * @param epsilon Simplification tolerance.
     * @return void
     */
    void simplifyRegionContours(Element::Region& region, float epsilon);

    /**
     * @brief Triangulates simplified region polygons.
     * @param region Region to mutate.
     * @return void
     */
    void triangulateRegion(Element::Region& region);

    /**
     * @brief Assigns Box2D world and scaling used by physics sync.
     * @param worldId Box2D world id.
     * @param pixelsPerMeter Scale factor between grid cells and world units.
     * @return void
     */
    void setPhysicsWorld(b2WorldId worldId, float pixelsPerMeter);

    /**
     * @brief Returns current world scaling factor.
     * @return Pixels-per-meter value.
     */
    float getPixelsPerMeter() const
    {
        return pixelsPerMeter;
    }
    
    bool tryDisplacePixel(int x, int y, int range);


private:
    uint64_t frame = 0;
    ChunkGrid &grid;
    std::vector<Element::Particle> particles;
    struct ChunkEntry
    {
        int    cx;
        int    cy;
        Chunk* chunk;
    };

    std::vector<ChunkEntry>      orderedChunks;
    std::vector<Element::Region> detectedRegions;

    std::unordered_set<int64_t>    visitedForRegions;
    b2WorldId                      physicsWorld   = b2_nullWorldId;
    float                          pixelsPerMeter = 1.0f;
    std::vector<b2BodyId>          regionBodies;
    std::vector<RegionBodyBinding> regionBodyBindings;
    bool                           regionsDirty = true;
};