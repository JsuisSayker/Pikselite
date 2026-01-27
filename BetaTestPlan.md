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

| **Feature ID** | **User Role**        | **Feature Name**   | **Short Description**                                  |
| -------------- | -------------------- | ------------------ | ------------------------------------------------------ |
| F1             | Game Editor          | Project Management | Create a new project or open an existing one           |
| F2             | Game Editor          | Sprite Creation    | Create and edit sprites using the sprite editor        |
| F3             | Game Editor          | Pixel Attributes   | Assign attributes (solid, liquid, gas, etc.) to pixels |
| F4             | Game Editor          | Pixel Simulation   | Simulate pixel physics and interactions                |
| F5             | Game Editor          | Game Editing       | Add objects and configure events using Blueprint       |
| F6             | Game Editor          | Asset Management   | Manage sprites, scenes, and objects via asset explorer |
| F7             | Game Editor          | Game Preview       | Simulate gameplay in real time inside the editor       |
| F8             | Game Editor / Player | Game Build         | Generate and run a playable executable                 |

---

## **4. Success Criteria**

| **Feature ID** | **Key Success Criteria**             | **Indicator / Metric**              | **Result**  |
| -------------- | ------------------------------------ | ----------------------------------- | ----------- |
| F1             | Projects can be created and reopened | No errors, correct folder structure | In Progress |
| F2             | Sprites are editable and reusable    | Sprite renders correctly in scene   | In Progress |
| F3             | Pixel attributes persist correctly   | Attributes saved and reloaded       | In Progress |
| F4             | Pixel simulation behaves correctly   | Expected interactions observed      | In Progress |
| F5             | Blueprint events execute correctly   | Objects react during preview        | In Progress |
| F6             | Assets are manageable                | Assets accessible and usable        | In Progress |
| F7             | Preview is stable and accurate       | No crashes, correct behavior        | In Progress |
| F8             | Build produces a working game        | Executable launches and runs        | In Progress |