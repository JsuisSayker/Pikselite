# Pikselite Engine

A 2D pixel-based game engine featuring pixel physics simulation with marching squares triangulation, ECS architecture, Lua scripting, ImGui-based editors, Box2D physics, and Tracy profiling for Debug builds.

## Project Purpose

Pikselite Engine is a standalone game development framework designed for creating pixel-art style 2D games. It combines:

- **Pixel Physics Simulation**: Chunk-based grid system with element types (solid, liquid, gas) and marching squares triangulation for generating physics colliders from pixel shapes
- **Entity Component System (ECS)**: Lightweight ECS architecture with components (Transform, Velocity, Sprite, PhysicsBody) and systems (Movement, SpriteRender, Script, Physics)
- **Lua Scripting**: Game logic scripts via Lua/sol2 bindings
- **ImGui Editors**: Built-in Sprite Editor and Project Editor for creating and managing game content
- **Box2D Integration**: Full 2D physics engine for dynamic body simulation
- **Tracy Profiling**: Frame-by-frame profiling for performance analysis (Debug builds only)

## Install/Deploy

### Automated Setup

The project includes an automated setup script that installs all required dependencies:

```
setup.bat [Debug|Release]
```

The script automatically:
- Installs CMake v3.30.0 (portable) if not found
- Clones and bootstraps vcpkg if not present
- Installs Visual Studio 2022 Build Tools if missing
- Installs all dependencies via vcpkg
- Configures and builds the project
- Runs the executable

### Manual Build

```bash
# Install dependencies
vcpkg integrate install --triplet x64-windows

# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DVCPKG_OVERLAY_PORTS=external/overlay-ports -DVCPKG_TARGET_TRIPLET=x64-windows

# Build
cmake --build build --config Release
```

## Folder/Module Structure

```
Pikselite-Engine/
├── src/                    # Implementation files (.cpp)
│   ├── main.cpp            # Application entry point
│   ├── graphics/          # Renderer, ImGui, interface
│   │   ├── renderer/      # OpenGL rendering
│   │   ├── imgui/        # ImGui integration
│   │   └── interface/    # SDL2 windowing
│   ├── engine/           # Core engine
│   │   ├── ecs/         # Entity Component System
│   │   │   ├── components/  # Transform, Velocity, Sprite, Physics
│   │   │   └── systems/   # Movement, Physics, Script, SpriteRender
│   │   ├── pixels/       # Pixel simulation
│   │   │   └── simulation/ # Chunk grid, elements
│   │   ├── managers/     # Entity, Component, System, Lua managers
│   │   ├── physics/     # Box2D wrapper
│   │   └── events/      # Event bus system
│   └── editors/          # Built-in editors
│       ├── sprite/      # Sprite Editor
│       └── project/     # Project Editor
├── interface/             # Public headers (.hpp)
│   ├── engine/
│   ├── graphics/
│   └── editors/
├── external/              # Third-party files
│   ├── profiler/         # Tracy profiler executable
│   └── overlay-ports/   # Custom vcpkg ports
├── scripts/              # Lua scripts
│   └── movement.lua     # Example movement script
├── assets/               # Scene files, sprites
│   └── scene.json       # Default scene
├── tests/                # Unit tests (Google Test)
│   ├── engine/         # ECS and physics tests
│   ├── renderer/      # Rendering tests
│   ├── graphics/      # Window tests
│   └── imguiComponents/ # ImGui component tests
├── build.bat             # Build script
├── setup.bat            # Full setup + build + run
├── CMakeLists.txt       # CMake configuration
└── vcpkg.json          # Dependencies manifest
```

## Key Commands

| Command | Description |
|---------|-------------|
| `setup.bat` | Full setup + build + run (Release mode) |
| `setup.bat Debug` | Full setup + build + run (Debug mode) |
| `build.bat` | Build only (Release mode) |
| `build.bat Debug` | Build only (Debug mode) |

## Environment Variables

No environment variables are required. The build scripts automatically detect and install all dependencies.

## Technical Prerequisites

- **Operating System**: Windows 10/11 (x64)
- **Build Tools**: Visual Studio 2022 Build Tools with C++ workload
- **Version Control**: Git
- **Network**: Internet connection (for downloading CMake, vcpkg, VS Build Tools)

## Build System Details

| Component | Version | Notes |
|-----------|---------|-------|
| CMake | 3.20+ | Auto-installed v3.30.0 portable |
| vcpkg | Latest | Auto-installed if missing |
| Target Triplet | x64-windows | Windows x64 |
| C++ Standard | C++17 | Modern C++ |
| MSVC Toolset | v143 | Visual Studio 2022 |

### Build Configuration

The project uses CMake with vcpkg for dependency management. Key configuration options:

- `ENABLE_COVERAGE`: Enable code coverage reporting (default: ON)
- `PIKSELITE_ENABLE_PROFILING`: Enable Tracy profiling (default: ON, Debug only)
- `VCPKG_OVERLAY_PORTS`: Custom ports (external/overlay-ports)

## Dependencies

All dependencies are managed via vcpkg:

| Package | Purpose |
|---------|---------|
| sdl2 | Window creation and input handling |
| glew | OpenGL extension loading |
| opengl | Graphics rendering API |
| stb | Image and utility libraries |
| glm | Mathematics library (vectors, matrices) |
| lua | Lua scripting runtime |
| sol2 | C++ to Lua bindings |
| gtest | Google Test framework |
| box2d | 2D physics engine |
| nlohmann-json | JSON serialization |
| tracy (>=0.13.1) | Frame profiler |
| imgui-backends | ImGui with OpenGL3 and SDL2 bindings |

## Architecture Overview

### Entity Component System (ECS)

The engine implements a custom ECS architecture:

**Components:**
- `Transform`: Position (x, y), rotation, scale (scaleX, scaleY)
- `Velocity`: Velocity (vx, vy)
- `Sprite`: Width, height, texture reference
- `PhysicsBody`: Box2D body, body type (static/dynamic), density, friction, restitution, triangles
- `Script`: Per-entity Lua script attachment (`scriptPath`)

**Systems:**
- `MovementSystem`: Updates position based on velocity
- `SpriteRenderSystem`: Renders sprite components with camera
- `ScriptSystem`: Executes per-entity Lua scripts with callbacks and gameplay APIs
- `PhysicsSystem`: Box2D physics simulation with automatic collider generation

### Lua Scripting API (Runtime)

Entities/components:
- `get_entity(id)`, `get_entity_by_name(name)`, `get_entities()`
- `create_entity(optionalTable)`, `delete_entity(id)`
- `has_component(id, type)`, `get_component(id, type)`
- `set_component(id, type, table)`, `add_component(id, type, table)`, `remove_component(id, type)`

Gameplay:
- `create_pixel(x, y, type)`, `create_pixels({...})`
- `basic_chase(selfId, targetId, speed)`
- `set_victory(reason)`, `set_lose(reason)`, `is_victory()`, `is_lose()`

Optional script callbacks:
- `on_collision_enter(otherId)`
- `on_collision_exit(otherId)`
- `on_victory(reason)`
- `on_lose(reason)`

**Managers:**
- `EntityManager`: Entity allocation and destruction
- `ComponentManager`: Component storage and retrieval
- `SystemManager`: System updates and entity filtering

### Pixel Simulation

The pixel simulation system provides:

- **Chunk-based Grid**: 16x16 pixel chunks for efficient memory management
- **Element Types**: EMPTY, SOLID_STATIC, SOLID_DYNAMIC, LIQUID, GAS
- **Marching Squares**: Algorithm for generating polygon contours from pixel regions
- **Triangulation**: Converting contours to physics collider triangles
- **Box2D Integration**: Automatic physics body creation from pixel shapes

### Editors

Two built-in editors enable game content creation:

**Sprite Editor:**
- Draw pixels on a canvas
- Create pixel-based sprite definitions
- Zoom and pan navigation

**Project Editor:**
- Manage game objects
- Configure components (Transform, Velocity, Sprite, PhysicsBody)
- Edit chunk grid and elements
- Save/load scenes

## Running Tests

The project includes unit tests built with Google Test:

```bash
# Build tests
cmake --build build --config Release --target pikselite_tests

# Run tests
build\Release\pikselite_tests.exe

# Or use ctest
ctest --test-dir build --output-on-failure
```

### Test Categories

| Test File | Category |
|-----------|----------|
| testEcs.cpp | ECS component and system tests |
| testPhysics.cpp | Physics simulation tests |
| testChunk.cpp | Chunk grid tests |
| testDrawOnEditor.cpp | Editor rendering tests |
| testWindowSize.cpp | Window management tests |
| testBarComponent.cpp | ImGui bar component tests |

## Assets/Scene Format

Scenes are stored in JSON format (`assets/scene.json`):

```json
{
    "version": 1,
    "gameObjects": [
        {
            "id": 1,
            "name": "Player",
            "isActive": true,
            "pixels": [],
            "pixelLocalCoords": [],
            "components": {}
        }
    ],
    "gameObjectCounter": 1,
    "chunkGrid": {
        "chunks": {}
    },
    "renderPixels": [],
    "pixelAttributes": {
        "solidAttributes": [],
        "liquidAttributes": [],
        "gaseousAttributes": [],
        "renderIndex": []
    },
    "pixelIdCounter": 1
}
```

### Scene Fields

- `version`: Scene format version
- `gameObjects`: Array of game objects with pixels and components
- `gameObjectCounter`: Auto-incrementing ID counter
- `chunkGrid`: Chunk-based pixel grid storage
- `renderPixels`: Pre-rendered pixels for optimization
- `pixelAttributes`: Element type definitions and render indices
- `pixelIdCounter`: Auto-incrementing pixel ID counter

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `TAB` | Toggle between Sprite Editor and Project Editor |
| `F5` | Start Game Preview |
| `W` | Pan camera up |
| `A` | Pan camera left |
| `S` | Pan camera down |
| `D` | Pan camera right |
| `I` | Zoom in |
| `O` | Zoom out |
| `ESC` | Close window / Cancel operation |

## Profiling

Tracy profiler integration provides frame-by-frame performance analysis:

- **Enabled**: Debug builds only
- **Port**: Default listen on UDP port 8086
- **Launch**: Run `external/profiler/tracy-profiler.exe`

### Using Tracy

1. Build in Debug mode: `setup.bat Debug`
2. Launch the profiler: `external\profiler\tracy-profiler.exe`
3. Run the application
4. Connect to the profiler and view frame data

## License

See project repository for license information.