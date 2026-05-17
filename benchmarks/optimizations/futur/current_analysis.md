# Current Analysis — Pikselite-Engine

---

## Bottlenecks

### 1. Pixel Simulation — the #1 hot path

`element.cpp:383-433` runs every fixed timestep (60 Hz). For each frame:

- `resetUpdatedFlags()` (line 468) iterates **all pixels in all chunks**, even empty ones
- `orderChunksForUpdate()` (line 478) copies all chunk entries into a vector then `std::sort`s them — heap allocation + O(n log n) every tick
- Each non-empty pixel dispatches through a function pointer (`def.update`), preventing inlining
- `rand()` is called per-pixel for dispersion direction and fire spread — not thread-safe, high contention

At 200×200 = 40K pixels, this is 40K+ iterations × several passes per frame.

### 2. Marching Squares + Triangulation — all-or-nothing rebuild

`detectRegions()` (`element.cpp:539-629`) runs whenever `regionsDirty`:

- Destroys **all** existing Box2D bodies before rebuilding
- Flood fill → marching squares → RDP simplification → ear-clipping triangulation
- Ear-clipping (`triangulateRegion`, line 1056) has an O(n²) worst-case with a hard guard of 10K iterations
- `visitedForRegions` `unordered_set` is rebuilt from scratch each time
- No incremental update — one pixel change triggers a full region rebuild

### 3. `buildRenderPixels()` — redundant pixel marshaling

`core.cpp:495-558` converts the entire simulation grid → `std::vector<graphics::Pixel>` every frame. Each pixel does:

- `(cx * 32 + x) * PIXEL_SIZE` float math
- Index into `colorPalette` with float division by 255.0
- If burning: two color lookups + `glm::mix` + `glm::clamp`
- `push_back` into a vector (reallocations: `reserve(10000)` is a guess, not based on actual count)

This is done **unconditionally**, even if nothing changed.

### 4. Double sprite iteration

`drawSpritesBelowLayer` + `drawSpritesAboveLayer` (`core.cpp:891-1021`) are ~95% duplicate code that both:

- Linear scan all entities (5000 max)
- Filter, sort by layer, then render
- No spatial culling, no caching

### 5. Lua → C++ double marshaling

`ScriptSystem::update()` (`scriptSystem.hpp:108-154`) per-entity per-frame:

- `buildEntityTable()` copies C++ component data into a new Lua table
- Lua `update(dt)` runs user code that modifies the table
- `applyEntityTableChanges()` reads the table back into C++ components
- This round-trips all component fields even if only one changed

### 6. Build pipeline I/O

`core.cpp:560-747` uses `_popen` with tiny 128-byte buffers, reconfigures CMake from scratch for every build, and blocks a thread on synchronous process I/O.

---

## Friction Points

| Issue                                          | Location                                                   | Impact                                                                                                  |
| ---------------------------------------------- | ---------------------------------------------------------- | ------------------------------------------------------------------------------------------------------- |
| **ScriptSystem is 1133-line header-only file** | `scriptSystem.hpp`                                         | Long incremental rebuilds; drags in Lua, SDL, glm, all component headers into every TU that includes it |
| **`.env` contains GITHUB_TOKEN**               | `.env`                                                     | Security leak if committed/pushed                                                                       |
| **Disabled tests**                             | `tests/CMakeLists.txt` — 4 .cpp files commented out        | Test coverage gaps; likely rotted                                                                       |
| **TODO stubs**                                 | `core.cpp:33-41` — `gameObjectToJson`/`gameObjectFromJson` | Incomplete scene serialization path                                                                     |
| **Hardcoded absolute path**                    | `config/info.json`                                         | Doesn't work on other machines                                                                          |
| **`rand()` everywhere**                        | `element.cpp` passim                                       | Not thread-safe, poor distribution; prevents parallel simulation                                        |
| **Duplicate sprite draw code**                 | `core.cpp` lines 891-955 vs 957-1021                       | ~130 lines of near-identical logic; bugs fixed in one won't be in the other                             |
| **No vcpkg version pins**                      | `vcpkg.json`                                               | Upstream updates can break builds                                                                       |
| **Build logic triplicated**                    | `setup.bat`, `build.bat`, CI YAML                          | Setup drift                                                                                             |

---

## Trade-off Justifications

### Quality vs. Performance

| Trade-off               | Choice                                 | Rationale                                                                                                                       |
| ----------------------- | -------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| Region detection        | Full rebuild vs. incremental           | Full rebuild is simpler to implement correctly; for the target game scale (small pixel worlds), the O(n²) ear-clip is tolerable |
| Pixel grid              | `unordered_map` chunks vs. dense array | Sparse worlds benefit from chunk-based storage; hash lookups per pixel are the cost of not needing a fixed world size           |
| Collision pipeline      | MS → RDP → ear-clip in one pass        | Off-the-shelf algorithms, no external dependency (no poly2tri, no CGAL); easy to debug                                          |
| `rand()` vs. `<random>` | `rand()`                               | Simple, deterministic per-seed; adequate for pixel simulation randomness; avoids complexity of proper RNG seeding/threading     |
| Fixed timestep cap      | 0.25s accumulator clamp                | Prevents spiral-of-death; simple to understand; trade-off: physics skips if frame rate tanks                                    |

### Performance vs. Cost (Development Complexity)

| Trade-off                     | Choice                    | Rationale                                                                                                                           |
| ----------------------------- | ------------------------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| ScriptSystem header-only      | No .cpp split             | Avoids managing a separate compilation unit, forward declarations, and linker symbols for what is essentially a single-class system |
| `std::any` components         | Type-erased storage       | Flexible component registration without code generation; no macros/templates for component registration                             |
| `buildRenderPixels` per frame | No dirty tracking         | Keeping a clean/dirty bitmap adds complexity; for small grids the full rebuild is fast enough                                       |
| `_popen` for build            | No proper process library | Cross-platform process spawning is complex; `_popen` works on Windows and keeps the build pipeline ~50 lines                        |
| No spatial index for sprites  | Linear scan + sort        | For <1000 sprites the O(n log n) sort dominates; a spatial hash would add ~200 lines with marginal gain at target scale             |

### Time vs. Quality

| Trade-off                     | Choice                                    | Rationale                                                                                                                    |
| ----------------------------- | ----------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| Tracy profiler always present | `#error` if disabled                      | Forces profiling on in debug; the developer values observability over compile-time flexibility                               |
| Test suite scope              | 18 test files with load/stress/resilience | High investment in correctness for the simulation and ECS — the two most complex subsystems; fewer tests for UI/editors      |
| No incremental compilation    | Full CMake reconfigure per build          | Build pipeline is a late addition; the `_popen + cmake --build` pattern works but doesn't use CMake's server mode or presets |
| Commented-out tests           | Abandoned rather than deleted             | Preserves history of what was intended; risk: they won't compile if the API changes                                          |

### Cost (Real-world Resource Trade-offs)

| Resource                         | Observation                                                                                                    |
| -------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| **vcpkg binary cache**           | CI uses `VCPKG_BINARY_SOURCES` with NuGet — avoids recompiling dependencies                                    |
| **Self-hosted Windows runners**  | Free CI minutes; but Windows-only, no macOS/Linux coverage                                                     |
| **Header-only libraries**        | sol2, glm, nlohmann-json, stb — zero build cost for dependencies; compile-time cost shifts to the engine's TUs |
| **Single monolithic editor exe** | No DLL/shared library split; simpler deployment, faster link, but larger binary                                |
| **Tracy**                        | Free profiler with high value; the `TracyClient.dll` copy step is a minor CI cost                              |

---

**Summary:** The engine's bottlenecks are in the pixel simulation loop (per-pixel function pointer dispatch, full-grid flag reset, chunk sorting) and the collision rebuild pipeline (all-or-nothing). The biggest friction points are the 1133-line header-only ScriptSystem (compile time) and the `rand()` usage (prevents parallel sim). The trade-offs consistently favor **implementation simplicity and debuggability** over peak performance — reasonable for a 2D pixel engine targeting small-to-medium game worlds.

Analysis done the 17/06/2026
