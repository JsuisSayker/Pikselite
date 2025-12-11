### **BETA TEST PLAN – Pikselite Engine**

## **1. Core Functionalities for Beta Version**
Below are the essential features that must be available for beta testing, along with any changes made since the initial Tech3 Action Plan.
| **Feature Name**  | **Description** | **Priority (High/Medium/Low)** | **Changes Since Tech3** |
|-------------------|---------------|--------------------------------|--------------------------|
|Project | The user can create a project or launch one that already exists. | Medium | - |
| Sprite Creation  | The user can create a sprite from the sprite editor and import it in the scene. | High | - |
| Pixels Attributes     | The user can add to any pixels inside a sprite an attribute | High | Refined how to simulate them. |
| Game editing     | The user can edit his game to add Object and Event | High | Refined event and input using Blueprint instead of scripting |
| Game preview     | The game has a preview to simulate the game and see his behavior. | High | - |
| Game Build     | The user can generate an executable game and launch it with it. | Medium | - |
## **2. Beta Testing Scenarios**

### **2.1 User Roles**
The following role will be involved in beta testing.

| **Role Name** | **Description** |
|----------------|-----------------|
| Game Editor    | Uses the game engine to create games |
| Player    | Launch the game and playe it |

### **2.2 Test Scenarios**

---

#### **Scenario 1: Project Management (Create or Open Project)**
- **Role Involved:** Game Editor  
- **Objective:** Validate project creation, loading existing projects, and correct directory initialization.  
- **Preconditions:**  
  - Pikselite Engine installed  
  - Directory available for new project  
  - Existing project folder available  

- **Test Steps:**
  1. Launch Pikselite Engine.
  2. Select "Create New Project" and provide name and folder path.
  3. Confirm project loads into the editor.
  4. Close and relaunch the engine.
  5. Select "Open Existing Project" and choose the previously created project.

- **Expected Outcome:**  
  - New project structure is created correctly.  
  - Project loads without errors.  
  - Existing project restores all assets successfully.

---

#### **Scenario 2: Sprite Creation**
- **Role Involved:** Game Editor  
- **Objective:** Ensure a user can create, edit, and import sprites into the scene.  
- **Preconditions:** A project is already created or opened.

- **Test Steps:**
  1. Open the Sprite Editor.
  2. Create a new sprite with defined name and dimensions.
  3. Draw pixels using brush, eraser, and color tools.
  4. Save the sprite.
  5. Go back to the main editor and import the saved sprite into the scene.

- **Expected Outcome:**  
  - Sprite is saved correctly with all pixel data.  
  - Sprite appears in the scene without corruption or import errors.

---

#### **Scenario 3: Pixel Attributes**
- **Role Involved:** Game Editor  
- **Objective:** Validate assignment and persistence of pixel attributes.  
- **Preconditions:** A sprite already exists.

- **Test Steps:**
  1. Open the Sprite Editor and load an existing sprite.
  2. Select a pixel and assign the "Solid" attribute.
  3. Assign the "Liquid" attribute and set viscosity on multiple pixels.
  4. Assign "Gas" attributes to others.
  5. Save and reopen the sprite.

- **Expected Outcome:**  
  - All attributes persist correctly after saving/reloading.  
  - Attribute parameters (e.g., viscosity) are preserved.  
  - Editor does not crash when mixing attribute types.
---

#### **Scenario 4: Game Editing (Objects & Events)**
- **Role Involved:** Game Editor  
- **Objective:** Validate that the user can add objects to the game scene and configure events using the Blueprint system.  
- **Preconditions:**  
  - A project with at least one scene is already created or opened.  
  - Blueprint system is enabled in the editor.  

- **Test Steps:**  
  1. Open the main scene editor.  
  2. Add a new object to the scene (e.g., entity, sprite instance, or interactive object).  
  3. Select the object and open its Blueprint editor.  
  4. Add an event node (e.g., OnStart, OnUpdate, OnCollision).  
  5. Add action nodes (e.g., Move, Change Attribute, Spawn Object) and link them to the event.  
  6. Validate the Blueprint graph for errors.  
  7. Save the scene and enter Preview mode.  
  8. Confirm that the object responds to the configured event during simulation.  

- **Expected Outcome:**  
  - Objects can be added to the scene without errors.  
  - Blueprint nodes function correctly and link without validation errors.  
  - Events and actions trigger during Preview mode exactly as configured.  
  - All modifications persist after saving and reopening the project.
---

#### **Scenario 5: Game Preview**
- **Role Involved:** Game Editor  
- **Objective:** Validate correct simulation behavior during preview mode.  
- **Preconditions:** A scene containing sprites with assigned pixel attributes.

- **Test Steps:**
  1. Press the "Preview" button.
  2. Observe sprite behavior and attribute-based interactions (solid, liquid, gas).
  3. Move sprites or trigger interactions according to gameplay rules.
  4. Exit the preview mode.

- **Expected Outcome:**  
  - Preview mode runs without performance drops or crashes.  
  - Pixel attributes behave consistently (liquids flow, solids stay static, gases disperse).  
  - Exiting preview restores editor mode cleanly.

---

#### **Scenario 6: Game Build**
- **Role Involved:** Game Editor and player
- **Objective:** Validate executable game build and correct execution.  
- **Preconditions:** A project with at least one scene.

- **Test Steps:**
  1. Open the Build menu.
  2. Select build options and platform.
  3. Click "Build Game".
  4. Wait for build to complete.
  5. Open output folder and launch the generated executable.

- **Expected Outcome:**  
  - Build completes successfully.  
  - Executable launches without errors.  
  - Game logic matches behavior observed in preview mode.

---

## **3. Success Criteria**
- At least **90% of test scenarios must pass** without blocking issues.  
- No critical crashes occur during major workflows (project creation, sprite editing, preview, build).  
- Sprite manipulation, attribute assignment, and physics simulation behave as intended.  
- Testers report satisfactory usability and stability for prototyping.

---

## **4. Known Issues & Limitations**

| **Issue** | **Description** | **Impact** | **Planned Fix? (Yes/No)** |
|-----------|------------------|-----------|---------------------------|
| Pixel physics instability | Interaction between to much attributes can create inconsistency. | High | Yes |
| Missing undo/redo | No undo/redo in the Sprite Editor. | Medium | Yes |
| Limited build targets | Only one build platform supported in beta. | Low | No |
| Performance issues on large amount of pixels | Create lag issue  | Medium | Yes |

---

## **5. Conclusion**
This Beta Test Plan ensures that all core functionalities of the Pikselite Engine are validated under realistic user workflows. The testing phase aims to confirm stability, usability, and correctness before advancing to broader testing and public release.