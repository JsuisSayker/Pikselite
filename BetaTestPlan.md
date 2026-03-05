# **BETA TEST PLAN – Pikselite Engine**

## **1. Project Context**

Pikselite Engine is a 2D pixel-based game engine focused on fine-grained pixel manipulation and simulation.
It allows users to create projects, design sprites at the pixel level, assign physical and logical attributes to pixels, and simulate their interactions in real time.

The engine provides:

- A sprite editor with per-pixel attributes
- A Blueprint-based event system (no scripting required)
- A real-time preview mode
- A build system to generate playable executables

The goal of this beta test is to validate the stability, usability, and correctness of the core editor workflows before public release.

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

| **Feature ID** | **User Role**        | **Feature Name**   | **Short Description**                            |
| -------------- | -------------------- | ------------------ | ------------------------------------------------ |
| F1             | Game Editor          | Project Management | Create a new project                             |
| F2             | Game Editor          | Project Management | Save a project                                   |
| F3             | Game Editor          | Project Management | Open an existing project                         |
| F4             | Game Editor          | Sprite Creation    | Create sprites using the sprite editor           |
| F5             | Game Editor          | Sprite Creation    | Edit sprites using the sprite editor             |
| F6             | Game Editor          | Sprite Creation    | Save sprites using the sprite editor             |
| F7             | Game Editor          | Sprite Creation    | Load sprites using the sprite editor             |
| F8             | Game Editor          | Pixel Attributes   | Assign solid attribute to pixels                 |
| F9             | Game Editor          | Pixel Attributes   | Assign liquid attribute to pixels                |
| F11            | Game Editor          | Pixel Attributes   | Assign attribute gasness to pixels               |
| F12            | Game Editor          | Pixel Attributes   | Assign attribute flammable to pixels             |
| F13            | Game Editor          | Pixel Attributes   | Assign sand attribute to pixels                  |
| F14            | Game Editor          | Pixel Simulation   | Simulate pixel physics                           |
| F15            | Game Editor          | Pixel Simulation   | Simulate pixel interactions                      |
| F16            | Game Editor          | Game Editing       | Add objects to current scene                     |
| F17            | Game Editor          | Game Editing       | Configure events using scripting                 |
| F18            | Game Editor          | Asset Management   | Manage sprites via asset explorer                |
| F19            | Game Editor          | Asset Management   | Manage scenes via asset explorer                 |
| F20            | Game Editor          | Asset Management   | Manage objects via asset explorer                |
| F21            | Game Editor          | Game Preview       | Simulate gameplay in real time inside the editor |
| F22            | Game Editor / Player | Game Build         | Generate a playable executable                   |

---

## **4. Success Criteria**

| **Feature ID** | **Key Success Criteria**                                                    | **Indicator / Metric**                                                   | **Result**  |
| -------------- | --------------------------------------------------------------------------- | ------------------------------------------------------------------------ | ----------- |
| F1             | Projects can be created                                                     | No errors, correct folder structure                                      | In Progress |
| F2             | Projects can be saved                                                       | No errors, correct folder structure, and the changes are correctly saved | In Progress |
| F3             | Projects can be reopened                                                    | No errors, correct folder structure                                      | In Progress |
| F4             | Sprites can be created                                                      | Sprite appear in sprite editor                                           | In Progress |
| F5             | Sprites are editable                                                        | Sprite can be modified in sprite editor                                  | In Progress |
| F6             | Sprites can be saved                                                        | Sprite are correctly saved in the wanted location                        | In Progress |
| F7             | Sprites can be loaded in scene                                              | Sprite renders correctly in scene, and attributes are preserved          | In Progress |
| F8             | Pixel with solid attribute persist correctly                                | Attributes saved and reloaded                                            | In Progress |
| F9             | Pixel with liquid attribute persist correctly                               | Expected interactions observed                                           | In Progress |
| F10            | Pixel with gas attribute persist correctly                                  | Expected interactions observed                                           | In Progress |
| F11            | Pixel with flammable attribute persist correctly                            | Expected interactions observed                                           | In Prgress  |
| F12            | Pixel with sand attribute persist correctly                                 | Expected interactions observed                                           | In Progress |
| F13            | Pixel with sand attribute persist correctly                                 | Expected interactions observed                                           | In Progress |
| F14            | Physics of different type pixels behaves correctly                          | Objects react during preview                                             | In Progress |
| F15            | Interaction between different pixels behaves correctly                      | Objects react during preview                                             | In Progress |
| F16            | Can add an object to the current scene                                      | Objects react during preview                                             | In Progress |
| F17            | An action like the player's movement can be performed by the scripting      | Objects react during preview                                             | In Progress |
| F18            | Can move and modify sprites using the assets explorer                       | Objects react during preview                                             | In Progress |
| F19            | Can move and modify scenes using the assets explorer                        | Objects react during preview                                             | In Progress |
| F20            | Can move and modify objects using the assets explorer                       | Objects react during preview                                             | In Progress |
| F21            | A preview can be launched from the editor, it should be stable and accurate | Assets accessible and usable                                             | In Progress |
| F22            | Build produces a working game                                               | Executable launches and runs                                             | In Progress |
