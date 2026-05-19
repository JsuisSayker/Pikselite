# **BETA TEST PLAN – Pikselite Engine**

## **1. Project Context**

Pikselite Engine is a 2D pixel-based game engine focused on fine-grained pixel manipulation and simulation.
It allows users to create projects, design sprites at the pixel level, using elements like water and fire, and simulate their interactions in real time.

The engine provides:

- A sprite editor with pixel Elements
- A game editor to create the game
- A Scripting-based event system
- A real-time preview mode
- A build system to generate playable executables

The goal of this beta test is to validate the stability, usability, and correctness of the core editor workflows before public release.

**Definitions:**
 - Game Object: Entity in the game that can be linked using component.
 - Sprite: Graphical element that can be moved in the renderer.
 - PNG sprite: Graphical element using PNG images.
 - Pixel sprite: Graphical element composed of pixel elements.

---

## **2. User Roles**

The following roles will be involved in beta testing.

| **Role Name** | **Description**                                                         |
| ------------- | ----------------------------------------------------------------------- |
| Game Editor   | Uses the engine to create projects, sprites, scenes, and gameplay logic |
| Player        | Launches the built game and plays it                                    |

---

## **3. Feature Table**

The following features will be shown and validated during the defense.

| **Feature ID** | **User Role**        | **Feature Name**       | **Short Description**                                             |
| -------------- | -------------------- | ---------------------- | ----------------------------------------------------------------- |
| F1             | Game Editor          | Project Management     | Create a new project                                              |
| F2             | Game Editor          | Project Management     | Save a project                                                    |
| F3             | Game Editor          | Project Management     | Open an existing project                                          |
| F4             | Game Editor          | Project Management     | Import a project                                                  |
| F5             | Game Editor          | Pixel sprite Creation | Create Pixel sprites using the sprite editor                     |
| F6             | Game Editor          | Pixel sprite Creation | Edit Pixel sprites using the sprite editor                       |
| F7             | Game Editor          | Pixel sprite Creation | Save Pixel sprites using the sprite editor                       |
| F8             | Game Editor          | Pixel sprite Creation | Load Pixel sprites using the sprite editor                       |
| F9             | Game Editor          | Pixel sprite Creation | Remove pixels using the eraser                                    |
| F10            | Game Editor          | Pixel sprite Creation | Switch pixel element brush                                        |
| F11            | Game Editor          | Pixel sprite Creation | Resize the brush                                                  |
| F12            | Game Editor          | Pixel Elements         | Place Stone element                                               |
| F13            | Game Editor          | Pixel Elements         | Place Dirt element                                                |
| F14            | Game Editor          | Pixel Elements         | Place Wood element                                                |
| F15            | Game Editor          | Pixel Elements         | Place Sand element                                                |
| F16            | Game Editor          | Pixel Elements         | Place Water element                                               |
| F17            | Game Editor          | Pixel Elements         | Place Lava element                                                |
| F18            | Game Editor          | Pixel Elements         | Place Fire element                                                |
| F19            | Game Editor          | Game Editing           | Create PNG sprite game object to current scene                    |
| F20            | Game Editor          | Game Editing           | Create Pixel sprite game object to current scene                 |
| F21            | Game Editor          | Game Editing           | Rename a game object                                              |
| F22            | Game Editor          | Game Editing           | Remove a game object                                              |
| F23            | Game Editor          | Game Editing           | Move a game object                                                |
| F24            | Game Editor          | Game Editing           | Add a physics component to an existing game object                |
| F25            | Game Editor          | Game Editing           | Add gravity force to a physics game object                        |
| F26            | Game Editor          | Game Editing           | Add rotation locker to the a physics game object                  |
| F27            | Game Editor          | Game Editing           | Remove a physics component to an existing game object             |
| F28            | Game Editor          | Game Editing           | Add a velocity component to an existing game object               |
| F29            | Game Editor          | Game Editing           | Remove a velocity component to an existing PNG sprite game object |
| F30            | Game Editor          | Game Editing           | Add a sprite component to an existing game object                 |
| F31            | Game Editor          | Game Editing           | Resize a sprite game object using sprite component                |
| F32            | Game Editor          | Game Editing           | Change renderer layer of a sprite game object                     |
| F33            | Game Editor          | Game Editing           | Remove a sprite component to an existing game object              |
| F34            | Game Editor          | Game Editing           | Add a script component to an existing game object                 |
| F35            | Game Editor          | Game Editing           | Remove a script component to an existing game object              |
| F36            | Game Editor          | Game Editing           | Configure sprite game object events using scripting               |
| F37            | Game Editor          | Asset Management       | Manage scenes via asset explorer                                  |
| F38            | Game Editor          | Game Preview           | Simulate gameplay in real time inside the editor                  |
| F39            | Game Editor          | Pixel Simulation       | Simulate pixel physics                                            |
| F40            | Game Editor          | Pixel Simulation       | Simulate pixel interactions                                       |
| F41            | Game Editor / Player | Game Build             | Generate a playable executable                                    |

---

## **4. Success Criteria**

| **Feature ID** | **Key Success Criteria**                                      | **Indicator / Metric**                                             | **Result**  |
| -------------- | ------------------------------------------------------------- | ------------------------------------------------------------------ | ----------- |
| F1             | Projects can be created                                       | No errors, correct folder structure generated                      | Done        |
| F2             | Projects can be saved                                         | Changes are correctly saved without corruption                     | Done        |
| F3             | Existing projects can be opened                               | Project loads successfully with all assets restored                | Done        |
| F4             | Projects can be imported from another folder                  | Imported project structure and assets are preserved                | Done        |
| F5             | Pixel sprites can be created                                 | Pixel sprite appears correctly in sprite editor                   | Done        |
| F6             | Pixel sprites can be edited                                  | Pixel modifications are applied correctly                          | Done        |
| F7             | Pixel sprites can be saved                                   | Pixel sprite file is correctly stored and reusable                | Done        |
| F8             | Pixel Sprites can be loaded                                  | Sprite renders correctly with preserved Elements                   | Done        |
| F9             | Pixels can be erased using the eraser tool                    | Selected pixels are removed correctly                              | Done        |
| F10            | Pixel element brush can be switched                           | The brush draws the element choosen                                | Done        |
| F11            | Brush size can be resized                                     | Brush size updates correctly and affects drawing area              | Done        |
| F12            | Stone elements can be placed                                  | Stone pixels are created and behave as solid elements              | Done        |
| F13            | Dirt elements can be placed                                   | Dirt pixels are created and rendered correctly                     | Done        |
| F14            | Wood elements can be placed                                   | Wood pixels are created and support flammable interactions         | Done        |
| F15            | Sand elements can be placed                                   | Sand pixels fall and accumulate correctly                          | Done        |
| F16            | Water elements can be placed                                  | Water pixels flow correctly during simulation                      | Done        |
| F17            | Lava elements can be placed                                   | Lava interacts correctly with surrounding elements                 | Done        |
| F18            | Fire elements can be placed                                   | Fire spreads and disappears according to simulation rules          | Done        |
| F19            | PNG prite game objects can be created in the current scene    | Game object appears correctly in scene and viewport                | Done        |
| F20            | Pixel sprite game objects can be created in the current scene | Pixel object is added correctly to the scene                       | Done        |
| F21            | Game objects can be renamed                                   | New object name is updated and persisted correctly                 | Done        |
| F22            | Game objects can be removed                                   | Object disappears from scene without errors                        | In Progress |
| F23            | Game objects can be moved                                     | Object position updates correctly in scene                         | In Progress |
| F24            | Physics components can be added to objects                    | Physics behavior is enabled correctly                              | Done        |
| F25            | Gravity force can be added to physics objects                 | Object reacts correctly to gravity simulation                      | Done        |
| F26            | Rotation lock can be added to physics objects                 | Object rotation remains constrained during physics simulation      | Done        |
| F27            | Physics components can be removed                             | Object no longer reacts to physics simulation                      | Done        |
| F28            | Velocity components can be added                              | Game Object movement velocity is applied correctly                 | Done        |
| F29            | Velocity components can be removed                            | Object stops using velocity-based movement                         | Done        |
| F30            | Sprite components can be added                                | Sprite renders correctly on the game object                        | In Progress |
| F31            | Sprite game objects can be resized                            | Sprite scaling updates visually and physically correctly           | Done        |
| F32            | Renderer layers can be changed                                | Rendering order updates correctly in scene                         | Done        |
| F33            | Sprite components can be removed                              | Sprite no longer renders on the object                             | Done        |
| F34            | Script components can be added                                | Scripts are attached and initialized correctly                     | Done        |
| F35            | Script components can be removed                              | Scripts no longer affect the game object                           | Done        |
| F36            | Sprite Game object events can be configured using scripting   | Scripted events trigger and execute correctly                      | Done        |
| F37            | Scenes can be managed in the asset explorer                   | Scenes can be created, renamed, moved, and deleted correctly       | Done        |
| F38            | Real-time preview can be launched inside the editor           | Preview runs stably and reflects current scene state               | Done        |
| F39            | Pixel physics simulation behaves correctly                    | Pixel movement and physics interactions are accurate               | Done        |
| F40            | Pixel interactions behave correctly                           | Elements interact according to defined simulation rules            | Done        |
| F41            | Playable executables can be generated                         | Build completes successfully and generated game launches correctly | Done        |
