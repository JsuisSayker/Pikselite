/** 
 * @file imguiInterface.cpp
 * @brief Implementation of the ImGui interface for the Pixel Engine.
 * This file contains the implementation for initializing and managing the ImGui interface within the application.
 */

#include <graphics/imgui/imguiInterface.hpp>
#include <nfd.hpp>

#include <filesystem>
#include <fstream>
#include <chrono>

namespace graphics {
    /** 
     * @brief Constructs an instance of the ImguiInterface.
     * @param window The SDL window to associate with the ImGui interface.
     * @param glContext The OpenGL context for rendering.
     */
    ImguiInterface::ImguiInterface(SDL_Window* window, SDL_GLContext glContext)
        : _window(window), _glContext(glContext) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        // ── Global Unity-like style overrides ─────────────────────────────────
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding    = 0.0f;
        style.ChildRounding     = 4.0f;
        style.FrameRounding     = 3.0f;
        style.GrabRounding      = 3.0f;
        style.PopupRounding     = 4.0f;
        style.ScrollbarRounding = 3.0f;
        style.TabRounding       = 4.0f;
        style.WindowBorderSize  = 1.0f;
        style.FrameBorderSize   = 0.0f;
        style.WindowPadding     = ImVec2(8.0f, 8.0f);
        style.FramePadding      = ImVec2(6.0f, 4.0f);
        style.ItemSpacing       = ImVec2(6.0f, 5.0f);
        style.ScrollbarSize     = 12.0f;
        style.GrabMinSize       = 8.0f;

        ImVec4* c = style.Colors;
        c[ImGuiCol_Text]                 = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        c[ImGuiCol_TextDisabled]         = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
        c[ImGuiCol_WindowBg]             = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        c[ImGuiCol_ChildBg]              = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        c[ImGuiCol_PopupBg]              = ImVec4(0.14f, 0.14f, 0.14f, 0.98f);
        c[ImGuiCol_Border]               = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        c[ImGuiCol_FrameBg]              = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_FrameBgHovered]       = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        c[ImGuiCol_FrameBgActive]        = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        c[ImGuiCol_TitleBg]              = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_TitleBgActive]        = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        c[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        c[ImGuiCol_CheckMark]            = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        c[ImGuiCol_SliderGrab]           = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        c[ImGuiCol_SliderGrabActive]     = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        c[ImGuiCol_Button]               = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
        c[ImGuiCol_ButtonHovered]        = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
        c[ImGuiCol_ButtonActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        c[ImGuiCol_Header]               = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        c[ImGuiCol_HeaderHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.50f);
        c[ImGuiCol_HeaderActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.85f);
        c[ImGuiCol_Separator]            = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        c[ImGuiCol_ResizeGrip]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        c[ImGuiCol_Tab]                  = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        c[ImGuiCol_TabHovered]           = ImVec4(0.26f, 0.59f, 0.98f, 0.50f);
        c[ImGuiCol_TabActive]            = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        c[ImGuiCol_PlotLines]            = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        c[ImGuiCol_PlotHistogram]        = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);

        ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        scanSprites();
    }

    /** 
     * @brief Destructor for the ImguiInterface class. Shuts down the ImGui context and cleans up resources.
     */
    ImguiInterface::~ImguiInterface() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    /** 
     * @brief Starts a new ImGui frame. This should be called at the beginning of each frame before any ImGui rendering calls.
     */
    void ImguiInterface::startFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    /** 
    * @brief Ends the current ImGui frame and renders the ImGui draw data. This should be called at the end of each frame after all ImGui rendering calls.
    * @param window The SDL window to render to.
    */
    void ImguiInterface::endFrame(SDL_Window* window) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    /** 
     * @brief Displays the ImGui demo window. This is a built-in ImGui feature that showcases various ImGui components and their usage.
     */
    void ImguiInterface::showImGuiDemo() {
        ImGui::Begin("Hello, ImGui!");
        ImGui::Text("This is a simple ImGui window.");
        ImGui::End();
    }

    /** 
     * @brief Displays the pixel editor sidebar for a given pixel, allowing users to modify its color and attributes. The sidebar includes options for adding or removing solid, liquid, and gaseous attributes, as well as a color picker for changing the pixel's color.
     * @param pixel The pixel to be edited.
     * @param grid The chunk grid containing the pixel.
     * @param pixelAttributes The attributes of the pixel.
     * @param label The label for the color picker.

     */
    void ImguiInterface::pixelEditor(Pixel& pixel, ChunkGrid grid, const char* label) {
        // TODO
    }

    /** 
     * @brief Displays the default pixel properties editor in a sidebar, allowing users to modify the default color and attributes for new pixels. The editor includes options for adding or removing solid, liquid, and gaseous attributes, as well as a color picker for changing the default pixel color.
     * @param defaultProperties The default pixel properties to be edited.
     * @param label The label for the color picker.
     */
    void ImguiInterface::defaultPixelElementEditor(Element::ElementType& elementType, const char* label) {
        const char* comboLabel = (label && label[0] != '\0') ? label : "Element Type";
        int current = static_cast<int>(elementType);

        auto currentName = [&]() -> const char* {
            if (current >= 0 && current < 256 && !g_elements[current].name.empty()) {
                return g_elements[current].name.c_str();
            }
            return "Unknown";
        };

        if (ImGui::BeginCombo(comboLabel, currentName())) {
            for (int i = 0; i < 256; ++i) {
                if (g_elements[i].name.empty()) continue; // only show registered elements

                const bool isSelected = (i == current);
                if (ImGui::Selectable(g_elements[i].name.c_str(), isSelected)) {
                    current = i;
                    elementType = static_cast<Element::ElementType>(i);
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    /** 
     * @brief Displays the pixel sprite handler sidebar, which provides options for editing pixel parameters and saving the current sprite. The sidebar includes a popup for entering the name of the sprite when saving.
     * @param showDefaultPropertiesEditor A reference to a boolean that indicates whether to show the default properties editor.
     * @param saveSpritePath A reference to a string that will hold the path where the sprite should be saved.
     */
    void ImguiInterface::pixelSpriteHandler(bool &showDefaultPropertiesEditor,
                                        std::string &saveSpritePath,
                                        Element::ElementType& selectedElementType)
{
    static BarConfig sideBarConfig {
        BarOrientation::Vertical,
        "Pixel Sprite Handler",
        ImVec2(LAYOUT_LEFT_W, 0.0f),
        true,
        GetDesiredPosition("left")
    };

    static Bar sideBar(sideBarConfig);

    sideBar.Draw([&]() {
        // Element dropdown directly in handler
        defaultPixelElementEditor(selectedElementType, "Element Type");

        if (BasicButton("Save Sprite")) {
            ImGui::OpenPopup("NameNewSpritePopup");
        };

        static char spriteName[128] = "";

        if (ImGui::BeginPopupModal("NameNewSpritePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter sprite name:");
            ImGui::InputText("Name##SpriteInput", spriteName, sizeof(spriteName));
            ImGui::Spacing();

            if (ImGui::Button("Save##SpriteButton", ImVec2(120, 0)))
            {
                std::string path = "assets/" + std::string(spriteName) + ".dat";
                saveSpritePath = path;
                spriteName[0] = '\0';
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel##SpriteButton", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    });
    }

    /** 
     * @brief Displays the top toolbar for the sprite editor, which includes options for selecting tools (paint or eraser) and adjusting brush size. The toolbar also manages the state of the selected tool and whether the eraser is active.
     * @param selectedTool A reference to an integer that indicates the currently selected tool (0 for paint, 1 for eraser).
     * @param brushSize A reference to an integer that represents the current brush size.
     * @param isEraserActive A reference to a boolean that indicates whether the eraser tool is currently active.
     */
    void ImguiInterface::spriteTopToolbar(int &selectedTool, int &brushSize, bool &isEraserActive)
    {
        static BarConfig topBarConfig {
            BarOrientation::Horizontal,
            "Sprite Tools",
            ImVec2(0.0f, LAYOUT_TOP_H),
            true,
            GetDesiredPosition("top")
        };

        static Bar topBar(topBarConfig);

        topBar.Draw([&]() {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Tool:");
            ImGui::SameLine();

            if (ImGui::Selectable("Paint", selectedTool == 0, 0, ImVec2(80, 0))) {
                selectedTool = 0;
                isEraserActive = false;
            }
            ImGui::SameLine();
            if (ImGui::Selectable("Eraser", selectedTool == 1, 0, ImVec2(80, 0))) {
                selectedTool = 1;
                isEraserActive = true;
            }

            ImGui::SameLine();
            ImGui::TextUnformatted("|");
            ImGui::SameLine();

            ImGui::SetNextItemWidth(180.0f);
            ImGui::SliderInt("Brush Size", &brushSize, 1, 8);

            if (selectedTool == 1) {
                isEraserActive = true;
            }
        });
    }

    /** 
     * @brief Displays an empty top toolbar for the project editor.
     */
    void ImguiInterface::projectTopBar(std::string title)
    {
        static BarConfig topBarConfig {
            BarOrientation::Horizontal,
            "Project Top Bar",
            ImVec2(0.0f, LAYOUT_TOP_H),
            true,
            GetDesiredPosition("top")
        };

        static Bar topBar(topBarConfig);
        topBar.Draw([&]() {

            if (BasicButton("Save")) {
                // 
            }

            ImVec2 textSize = ImGui::CalcTextSize(title.c_str());
            ImVec2 windowSize = ImGui::GetWindowSize();

            ImGui::SetCursorPos(ImVec2(
                (windowSize.x - textSize.x) * 0.5f,
                (windowSize.y - textSize.y) * 0.5f
            ));

            ImGui::Text("%s", title.c_str());
        });
    }

    /** 
     * @brief Displays a color selector widget in the ImGui interface, allowing users to pick a color. The function takes the current color as input and returns the selected color if it has been changed, or the original color if no change was made.
     * @param currentColor The current color to be displayed in the color picker.
     * @param label The label for the color picker widget.
     * @return The new color selected by the user, or the original color if no change was made.
     */
    glm::vec3 ImguiInterface::colorSelector(const glm::vec3& currentColor, const char* label) {
        float color[3] = { currentColor.r, currentColor.g, currentColor.b };
        if (ImGui::ColorPicker3(label, color)) {
            return glm::vec3(color[0], color[1], color[2]);
        }
        return currentColor;
    }

    namespace fs = std::filesystem;
    std::vector<std::string> spriteFiles;

    /** 
     * @brief Scans the "assets" directory for sprite files (PNG, JPG, DAT) and updates the list of available sprites. This function is called when the user clicks the "Refresh" button in the project navbar to ensure that any new or removed sprite files are reflected in the interface.
     */
    void ImguiInterface::scanSprites()
    {
        spriteFiles.clear();

        std::string folder = "assets";

        if (!fs::exists(folder))
        return;

        for (const auto& entry : fs::directory_iterator(folder))
        {
            if (entry.is_regular_file())
            {
                std::filesystem::path p = entry.path();
                std::string ext = p.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".webp" || ext == ".dat")
                {
                    spriteFiles.push_back(p.string());
                }
            }
        }
    }

    /** 
     * @brief Displays the project navbar, which includes a refresh button to scan for new sprites, buttons to save and load scenes, and a grid of available sprite thumbnails. When a sprite thumbnail is clicked, the current sprite filename is updated to reflect the selected sprite.
     * @param currentSpriteFilename A reference to a string that will hold the filename of the currently selected sprite.
     */
    void ImguiInterface::projectNavbar(std::string &currentSpriteFilename)
    {
        bool saveSceneRequested = false;
        bool loadSceneRequested = false;
        projectNavbar(currentSpriteFilename, saveSceneRequested, loadSceneRequested);
    }

    /** 
     * @brief Displays the project navbar with parameters for handling save and load scene requests. This function is an overload of the previous projectNavbar function, allowing the caller to also manage the state of save and load scene requests through boolean references.
     * @param currentSpriteFilename A reference to a string that will hold the filename of the currently selected sprite.
     * @param saveSceneRequested A reference to a boolean that indicates whether a save scene request has been made.
     * @param loadSceneRequested A reference to a boolean that indicates whether a load scene request has been made.
     */
    void ImguiInterface::projectNavbar(std::string &currentSpriteFilename, bool &saveSceneRequested, bool &loadSceneRequested)
    {
        static BarConfig bottomBarConfig {
            BarOrientation::Horizontal,
            "Project Navbar",
            ImVec2(0.0f, LAYOUT_BOTTOM_H),
            true,
            GetDesiredPosition("bottom")
        };

        static Bar bottomBar(bottomBarConfig);

        bottomBar.Draw([&]() {

            if (BasicButton("Refresh")) {
                scanSprites();
            }

            ImGui::SameLine();
            if (BasicButton("Save Scene")) {
                saveSceneRequested = true;
            }

            ImGui::SameLine();
            if (BasicButton("Load Scene")) {
                loadSceneRequested = true;
            }

            ImGui::Separator();

            float thumbnailSize = 65.0f;
            float padding = 15.0f;
            float cellSize = thumbnailSize + padding;

            float panelWidth = ImGui::GetContentRegionAvail().x;

            int columns = (int)(panelWidth / cellSize);
            if (columns < 1)
                columns = 1;

            ImGui::Columns(columns, 0, false);

           for (const auto& sprite : spriteFiles)
            {   
                ImGui::PushID(sprite.c_str());
            
                std::string name = std::filesystem::path(sprite).filename().string();
            
                ImGui::BeginGroup();
            
                float columnWidth = ImGui::GetColumnWidth();
                float offset = (columnWidth - thumbnailSize) * 0.5f;
            
                if (offset > 0)
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
            
                if (ImGui::Button("##thumb", ImVec2(thumbnailSize, thumbnailSize)))
                {
                    currentSpriteFilename = sprite;
                }
            
                float textWidth = ImGui::CalcTextSize(name.c_str()).x;
                float textOffset = (columnWidth - textWidth) * 0.5f;
            
                if (textOffset > 0)
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
            
                ImGui::TextWrapped("%s", name.c_str());
            
                ImGui::EndGroup();
            
                ImGui::PopID();
            
                ImGui::NextColumn();
            }

            ImGui::Columns(1);
        });
    }

    /** 
     * @brief Displays the game objects bar, which consists of a hierarchy view of all game objects and an inspector for the selected game object. The hierarchy allows users to select, rename, and delete game objects, while the inspector displays properties of the selected game object and allows users to edit them.
     * @param gameObjects A reference to a vector of game objects to be displayed in the hierarchy.
     * @param selectedGameObjectIndex A reference to an integer that indicates the index of the currently selected game object in the hierarchy.
     * @param currentProject A reference to the current project.
     */
    void ImguiInterface::gameObjectsBar(std::vector<::Pixel::GameObject>& gameObjects, int &selectedGameObjectIndex, const projects::Project& currentProject)
    {
        static BarConfig sideBarConfig {
            BarOrientation::Vertical,
            "Hierarchy",
            ImVec2(LAYOUT_LEFT_W, 0.0f),
            true,
            GetDesiredPosition("left")
        };
    
        static Bar sideBar(sideBarConfig);
    
        sideBar.Draw([&]() {
        
            static char searchBuffer[128] = "";
            static char renameBuffer[128] = "";
            static int renameIndex = -1;
            static bool openRenamePopup = false;
            static bool focusRename = true;
    
            ImGui::InputTextWithHint("##SearchObjects", "Search objects...", searchBuffer, sizeof(searchBuffer));
    
            std::string lowerSearch = searchBuffer;
            std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);
    
            if (ImGui::BeginChild("GameObjectList", ImVec2(0, 0), true))
            {
                for (size_t i = 0; i < gameObjects.size(); ++i)
                {
                    const std::string objName =
                        gameObjects[i].name.empty() ?
                        ("GameObject " + std::to_string(gameObjects[i].id)) :
                        gameObjects[i].name;
    
                    if (!lowerSearch.empty())
                    {
                        std::string lowerName = objName;
                        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
                        if (lowerName.find(lowerSearch) == std::string::npos)
                            continue;
                    }
    
                    ImGui::PushID((int)i);
    
                    if (ImGui::Selectable(objName.c_str(), selectedGameObjectIndex == (int)i))
                        selectedGameObjectIndex = (int)i;
    
                    // If right-clicked
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::MenuItem("Rename"))
                        {
                            strncpy(renameBuffer, objName.c_str(), sizeof(renameBuffer));
                            renameBuffer[sizeof(renameBuffer) - 1] = '\0';
    
                            renameIndex = (int)i;
                            openRenamePopup = true;
                        }
    
                        if (ImGui::MenuItem("Delete"))
                        {
                                if (selectedGameObjectIndex == (int)i)
                                    selectedGameObjectIndex = -1;
                                else if (selectedGameObjectIndex > (int)i)
                                    selectedGameObjectIndex--;
    
                            gameObjects.erase(gameObjects.begin() + i);
    
                            ImGui::EndPopup();
                            ImGui::PopID();
                            break;
                        }
    
                        ImGui::EndPopup();
                    }
    
                    ImGui::PopID();
                }
    
                ImGui::EndChild();
            }
    
            if (openRenamePopup)
            {
                ImGui::OpenPopup("RenameObjectPopup");
                openRenamePopup = false;
            }
    
            if (ImGui::BeginPopupModal("RenameObjectPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Rename Game Object");
    
                if (focusRename)
                {
                    ImGui::SetKeyboardFocusHere();
                    focusRename = false;
                }
    
                ImGui::InputText("New Name", renameBuffer, sizeof(renameBuffer));
    
                if (ImGui::Button("Save"))
                {
                    if (renameIndex >= 0 && renameIndex < (int)gameObjects.size())
                        gameObjects[renameIndex].name = renameBuffer;
    
                    renameBuffer[0] = '\0';
                    renameIndex = -1;
                    focusRename = true;
    
                    ImGui::CloseCurrentPopup();
                }
    
                ImGui::SameLine();
    
                if (ImGui::Button("Cancel"))
                {
                    renameIndex = -1;
                    focusRename = true;
    
                    ImGui::CloseCurrentPopup();
                }
    
                ImGui::EndPopup();
            }
    
        });

        static BarConfig inspectorBarConfig {
            BarOrientation::Vertical,
            "Inspector",
            ImVec2(LAYOUT_RIGHT_W, 0.0f),
            true,
            GetDesiredPosition("right")
        };

        static Bar inspectorBar(inspectorBarConfig);
        inspectorBar.Draw([&]() {
            if (selectedGameObjectIndex < 0 || selectedGameObjectIndex >= static_cast<int>(gameObjects.size())) {
                ImGui::TextDisabled("No GameObject selected");
                ImGui::Separator();
                ImGui::TextWrapped("Select a GameObject in the Hierarchy to edit its properties.");
                return;
            }

            auto &selected = gameObjects[selectedGameObjectIndex];

            ImGui::Separator();

            ImGui::Checkbox("Active", &selected.isActive);

            ImGui::SameLine();

            ImGui::Text("%s", selected.name.empty() ? ("GameObject " + std::to_string(selected.id)).c_str() : selected.name.c_str());

            ImGui::Separator();

            ImGui::TextDisabled("ID: %u", selected.id);
            ImGui::SameLine();
            ImGui::TextDisabled("Pixels: %d", static_cast<int>(selected.pixels.size()));

            ImGui::Separator();

            std::vector<std::string> availableComponents;

            if (!selected.hasComponent<ecs::components::Sprite>()) {
                availableComponents.push_back("Sprite");
            }  else {

                auto s = selected.getComponent<ecs::components::Sprite>();


                ImGui::Checkbox("##enabledSprite", &s->enabled);
                ImGui::SameLine();
                bool open = ImGui::CollapsingHeader("Sprite Component", nullptr, ImGuiTreeNodeFlags_DefaultOpen);

                if (open) {
                    ImGui::BeginDisabled(!s->enabled);

                    if (ImGui::Button("Reset##Sprite"))
                    {
                        // s.texturePath = "";
                        s->width = 640.0f;
                        s->height = 640.0f;
                    }

                    static char textureBuffer[256];
                    std::strncpy(textureBuffer, s->texturePath.c_str(), sizeof(textureBuffer));
                    if (ImGui::InputText("Sprite Texture Path", textureBuffer, sizeof(textureBuffer)))
                    {
                        s->texturePath = textureBuffer;
                    }
                    ImGui::DragFloat2("Width", &s->width, 640.0f);
                    ImGui::DragFloat2("Height", &s->height, 640.0f);

                    ImGui::EndDisabled();

                    if (ImGui::Button("Remove##Sprite"))
                    {
                        selected.removeComponent<ecs::components::Sprite>();
                    }
                }
            }

            if (!selected.hasComponent<ecs::components::Transform>()) {
                availableComponents.push_back("Transform");
            } else {
                
                auto t = selected.getComponent<ecs::components::Transform>();
                ImGui::Checkbox("##enabledTransform", &t->enabled);
                ImGui::SameLine();
                bool open = ImGui::CollapsingHeader("Transform Component", nullptr, ImGuiTreeNodeFlags_DefaultOpen);

                if (open) {
                    ImGui::BeginDisabled(!t->enabled);

                    if (ImGui::Button("Reset##Transform"))
                    {
                        t->x =0.0f;
                        t->y = 0.0f;
                        t->rotation = 0.0f;
                        t->scaleX = 1.0f;
                        t->scaleY = 1.0f;
                    }

                    ImGui::DragFloat2("Position", &t->x, 0.1f);
                    ImGui::DragFloat("Rotation", &t->rotation, 0.1f);
                    ImGui::DragFloat2("Scale", &t->scaleX, 0.1f);

                    ImGui::EndDisabled();
            
                    if (ImGui::Button("Remove##Transform"))
                    {
                        selected.removeComponent<ecs::components::Transform>();
                    }
                }
                
            }

            if (!selected.hasComponent<ecs::components::Velocity>()) {
                availableComponents.push_back("Velocity");
            } else {
                auto v = selected.getComponent<ecs::components::Velocity>();

                ImGui::Checkbox("##enabledVelocity", &v->enabled);
                ImGui::SameLine();
                bool open = ImGui::CollapsingHeader("Velocity Component", nullptr, ImGuiTreeNodeFlags_DefaultOpen);

                if (open) {
                    ImGui::BeginDisabled(!v->enabled);

                    if (ImGui::Button("Reset##Velocity"))
                    {
                        v->vx = 0.0f;
                        v->vy = 0.0f;
                    }
                
                    ImGui::DragFloat2("Velocity", &v->vx, 0.1f);
                    
                    ImGui::EndDisabled();
        
                    if (ImGui::Button("Remove##Velocity"))
                    {
                        selected.removeComponent<ecs::components::Velocity>();
                    }
                }
            }

            if (!selected.hasComponent<ecs::components::PhysicsBody>()) {
                availableComponents.push_back("PhysicsBody");
            } else {
                auto p = selected.getComponent<ecs::components::PhysicsBody>();

                ImGui::Checkbox("##enabledPhysics", &p->enabled);
                ImGui::SameLine();
                bool open = ImGui::CollapsingHeader("Physics Component", nullptr, ImGuiTreeNodeFlags_DefaultOpen);

                if (open) {
                    ImGui::BeginDisabled(!p->enabled);

                    if (ImGui::Button("Reset##Physics"))
                    {
                        p->bodyId = b2_nullBodyId;
                        p->bodyType = b2_dynamicBody;
                        p->fixedRotation = false;
                        p->density = 1.0f;
                        p->friction = 0.4f;
                        p->restitution = 0.1f;
                    }

                    const char* bodyTypes[] = {"Static", "Kinematic", "Dynamic"};
                    int bodyTypeIndex = 2;
                    if (p->bodyType == b2_staticBody) bodyTypeIndex = 0;
                    else if (p->bodyType == b2_kinematicBody) bodyTypeIndex = 1;

                    if (ImGui::Combo("Body Type", &bodyTypeIndex, bodyTypes, IM_ARRAYSIZE(bodyTypes)))
                    {
                        p->bodyType = bodyTypeIndex == 0 ? b2_staticBody : (bodyTypeIndex == 1 ? b2_kinematicBody : b2_dynamicBody);
                    }

                    ImGui::Checkbox("Fixed Rotation", &p->fixedRotation);
                    ImGui::DragFloat("Density", &p->density, 0.05f, 0.0f, 100.0f);
                    ImGui::DragFloat("Friction", &p->friction, 0.01f, 0.0f, 1.0f);
                    ImGui::DragFloat("Restitution", &p->restitution, 0.01f, 0.0f, 1.0f);

                    ImGui::TextDisabled("Triangles: %d", static_cast<int>(p->triangles.size()));
                    ImGui::TextDisabled("Body: %s", B2_IS_NULL(p->bodyId) ? "uncreated" : "created");

                    ImGui::EndDisabled();

                    if (ImGui::Button("Remove##Physics"))
                    {
                        selected.removeComponent<ecs::components::PhysicsBody>();
                    }
                }
            }
            
            PopupButton("Add Component", [&]() {
                static char search[64] = "";
                if (ImGui::IsWindowAppearing())
                {
                    search[0] = '\0';
                }
                ImGui::InputText("Search", search, IM_ARRAYSIZE(search));
                std::string query = search;
                std::transform(query.begin(), query.end(), query.begin(), ::tolower);

                for (const auto& comp : availableComponents) {
                    std::string lowerComp = comp;
                    std::transform(lowerComp.begin(), lowerComp.end(), lowerComp.begin(), ::tolower);

                    if (!query.empty() && lowerComp.find(query) == std::string::npos)
                        continue;

                    if (ImGui::Selectable(comp.c_str())) {
                        if (comp == "Sprite") {
                            selected.addComponent(ecs::components::Sprite{});
                            availableComponents.erase(std::remove(availableComponents.begin(), availableComponents.end(), "Sprite"), availableComponents.end());
                        } else if (comp == "Transform") {
                            selected.addComponent(ecs::components::Transform{});
                            availableComponents.erase(std::remove(availableComponents.begin(), availableComponents.end(), "Transform"), availableComponents.end());
                        } else if (comp == "Velocity") {
                            selected.addComponent(ecs::components::Velocity{});
                            availableComponents.erase(std::remove(availableComponents.begin(), availableComponents.end(), "Velocity"), availableComponents.end());
                        } else if (comp == "PhysicsBody") {
                            selected.addComponent(ecs::components::PhysicsBody{});
                            availableComponents.erase(std::remove(availableComponents.begin(), availableComponents.end(), "PhysicsBody"), availableComponents.end());
                        }
                        query.clear();
                        search[0] = '\0';
                    }
                }
            });
        });
    }

    void ImguiInterface::fileToolBar()
    {
        static BarConfig topBarConfig {
            BarOrientation::Horizontal,
            "File Toolbar",
            ImVec2(0.0f, LAYOUT_TOP_H),
            true,
            GetDesiredPosition("top")
        };

        static Bar topBar(topBarConfig);

        topBar.Draw([&]() {

            static int fileIndex = 0;
            std::vector<std::string> fileOptions = {"Save", "Exit"};
            if (DropdownButton("File", fileIndex, fileOptions))
            {
                switch (fileIndex)
                {
                    case 0:
                        // Save
                        break;
                    case 1:
                        // Exit
                        break;
                }
            }

            ImGui::SameLine();

            static int editIndex = 0;
            std::vector<std::string> editOptions = {"Placeholder1", "Placeholder2"};
            if (DropdownButton("Edit", editIndex, editOptions))
            {
                switch (editIndex)
                {
                    case 0:
                        // Placeholder1
                        break;
                    case 1:
                        // Placeholder2
                        break;
                }
            }

            ImGui::SameLine();

            if (BasicButton("Help"))
            {
                // TODO
            }
        });
    }

    int ImguiInterface::projectOptionsBar(std::vector<projects::Project> &projects)
    {
        float buttonHeight = 25.0f;
        float buttonWidth = 80.0f;
        float windowWidth = ImGui::GetContentRegionAvail().x;

        static bool openNewPopup = false;
        static bool openOpenPopup = false;
        static char projectName[128] = "NewProject";
        static std::filesystem::path selectedPath;
        int resultIndex = -1;

        ImGui::Text("Get Started");
        ImGui::Separator();

        if (BasicButton("New", buttonHeight, buttonWidth))
        {
            NFD::UniquePath outPath;

            nfdresult_t result = NFD::PickFolder(outPath);

            if (result == NFD_OKAY)
            {
                selectedPath = outPath.get();

                openNewPopup = true;
            } else if (result == NFD_ERROR)
            {
                std::cout << "Error: " << NFD::GetError() << std::endl;
                // std::cerr << "Error: " << NFD::GetError() << std::endl;
            }
        }

        if (openNewPopup)
        {
            ImGui::OpenPopup("Create Project");
            openNewPopup = false;
        }
        
        if (ImGui::BeginPopupModal("Create Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText("Project Name", projectName, sizeof(projectName));
        
            if (ImGui::Button("Save"))
            {
                std::filesystem::path fullPath = selectedPath / projectName;

                if (!std::filesystem::exists(fullPath))
                {
                    std::filesystem::create_directory(fullPath);
                    std::filesystem::create_directory(fullPath / "Assets");
                    std::filesystem::create_directory(fullPath / "Scenes");
                    std::ofstream(fullPath / "scene.json") << "{}";

                    projects::Project newProject;
                    newProject.name = projectName;
                    newProject.path = fullPath;

                    projects.push_back(newProject);

                    newProjectCreated = true;
                }

                ImGui::CloseCurrentPopup();
            }

            if (newProjectCreated)
            {
                newProjectCreated = false;
                resultIndex = projects.size() - 1;
            }
        
            ImGui::SameLine();
        
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }
        
            ImGui::EndPopup();
        }

        ImGui::SameLine();


        if (BasicButton("Open", buttonHeight, buttonWidth))
        {
            openOpenPopup = true;
        }

        if (openOpenPopup)
        {
            ImGui::OpenPopup("Open Project");
            openOpenPopup = false;
        }

        if (ImGui::BeginPopupModal("Open Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            for (int i = 0; i < static_cast<int>(projects.size()); i++)
            {
                clickableProjectOverview(projects[i]);
            
                if (ImGui::IsItemClicked())
                {
                    resultIndex = i;
                    
                    projects[i].lastOpened = std::chrono::system_clock::now();

                    ImGui::CloseCurrentPopup();
                    break;
                }
            }

            ImGui::EndPopup();
        }

        ImGui::SameLine();

        if (BasicButton("Import", buttonHeight, buttonWidth))
        {
            // TODO
            // return imported project;
        }

        ImGui::SameLine(windowWidth - buttonWidth);

        if (BasicButton("Tutorial", buttonHeight, buttonWidth))
        {
            // TODO
        }

        return resultIndex;
    }

    int ImguiInterface::recentProjectsDisplay(std::vector<projects::Project> &projects)
    {
        float buttonHeight = 20.0f;
        float buttonWidth = 80.0f;
        float windowWidth = ImGui::GetContentRegionAvail().x;

        ImGui::Text("Recent Projects");

        ImGui::SameLine();

        if (BasicButton("Refresh", buttonHeight, buttonWidth))
        {
            // TODO
        }

        ImGui::SameLine(windowWidth - buttonWidth);

        if (BasicButton("ListView", buttonHeight, buttonWidth))
        {
            // TODO
        }

        ImGui::SameLine();

        if (BasicButton("SquareView", buttonHeight, buttonWidth))
        {
            // TODO
        }

        if (projects.empty())
        {
            ImGui::Text("No recent projects found.");
            return -1;
        }

       for (int i = 0; i < 4 && i < static_cast<int>(projects.size()); i++)
        {
            clickableProjectOverview(projects[i]);
            
            if (ImGui::IsItemClicked())
            {
                projects[i].lastOpened = std::chrono::system_clock::now();
                return i;
            }
        }

        return -1;
    }

    void ImguiInterface::clickableProjectOverview(projects::Project &project)
    {
        // TODO: To the far right, 2 little buttons: for renaming, and for temp removing from recent projects list.
        
        //ImGui::Image((void*)(intptr_t)thumbnailTextureID, ImVec2(100, 100)); // thumbnail
        //ImGui::SameLine();

        ImGui::Text("%s", project.name.c_str());
        ImGui::Text("%s", project.path.string().c_str());

        std::time_t t = std::chrono::system_clock::to_time_t(project.lastOpened);
        ImGui::Text("Last opened: %s", std::ctime(&t));
    }

} // namespace graphics
