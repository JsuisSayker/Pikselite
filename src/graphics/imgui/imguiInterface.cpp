#include <graphics/imgui/imguiInterface.hpp>
#include <algorithm>
#include <string>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>

namespace graphics {
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

    ImguiInterface::~ImguiInterface() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void ImguiInterface::startFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void ImguiInterface::endFrame(SDL_Window* window) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImguiInterface::showImGuiDemo() {
        ImGui::Begin("Hello, ImGui!");
        ImGui::Text("This is a simple ImGui window.");
        ImGui::End();
    }

    void ImguiInterface::pixelEditor(Pixel& pixel, ::Pixel::ChunkGrid grid, ::Pixel::PixelAttributes &pixelAttributes, const char* label) {
        static BarConfig sideBarConfig {
            BarOrientation::Vertical,
            "Pixel Editor",
            ImVec2(LAYOUT_RIGHT_W, 0.0f),
            true,
            GetDesiredPosition("right")
        };

        static Bar sideBar(sideBarConfig);

        sideBar.Draw([&]() {
            float color[3] = { pixel.color.r, pixel.color.g, pixel.color.b };
            if (ImGui::ColorPicker3(label, color)) {
                pixel.color = glm::vec3(color[0], color[1], color[2]);
            }

            ::Pixel::PixelEntityID pixelId = grid.getPixel(pixel.position.x / PIXEL_SIZE, pixel.position.y / PIXEL_SIZE);

            std::vector<std::string> availableAttributes;

            auto s = pixelAttributes.solidAttributes.find(pixelId);
            if (s != pixelAttributes.solidAttributes.end()) {
                if (ImGui::CollapsingHeader("Solid")) {
                    if (ImGui::Button("Remove Attribute##Solid")) {
                        pixelAttributes.solidAttributes.erase(s);
                    }
                }
            } else {
                availableAttributes.push_back("Solid");
            }

            auto l = pixelAttributes.liquidAttributes.find(pixelId);
            if (l != pixelAttributes.liquidAttributes.end()) {
                if (ImGui::CollapsingHeader("Liquid")) {
                    ImGui::SliderFloat("Viscosity", &l->second.viscosity, 0.0f, 1.0f);
                    if (ImGui::Button("Remove Attribute##Liquid")) {
                        pixelAttributes.liquidAttributes.erase(l);
                    }
                }
            } else {
                availableAttributes.push_back("Liquid");
            }

            auto g = pixelAttributes.gaseousAttributes.find(pixelId);
            if (g != pixelAttributes.gaseousAttributes.end()) {
                if (ImGui::CollapsingHeader("Gaseous")) {
                    ImGui::SliderFloat("Density", &g->second.density, 0.0f, 1.0f);
                    if (ImGui::Button("Remove Attribute##Gaseous")) {
                        pixelAttributes.gaseousAttributes.erase(g);
                    }
                }
            } else {
                availableAttributes.push_back("Gaseous");
            }

            PopupButton("Add attribute", [&]() {
                static char search[64] = "";
                ImGui::InputText("Search", search, IM_ARRAYSIZE(search));
                std::string query = search;
                std::transform(query.begin(), query.end(), query.begin(), ::tolower);

                for (const auto& attr : availableAttributes) {
                    std::string lowerAttr = attr;
                    std::transform(lowerAttr.begin(), lowerAttr.end(), lowerAttr.begin(), ::tolower);

                    if (!query.empty() && lowerAttr.find(query) == std::string::npos)
                        continue;

                    if (ImGui::Selectable(attr.c_str())) {
                        if (attr == "Solid") {
                            pixelAttributes.solidAttributes[pixelId] = ::Pixel::Solid();
                            availableAttributes.erase(std::remove(availableAttributes.begin(), availableAttributes.end(), "Solid"), availableAttributes.end());
                        } else if (attr == "Liquid") {
                            pixelAttributes.liquidAttributes[pixelId] = ::Pixel::Liquid{0.5f};
                            availableAttributes.erase(std::remove(availableAttributes.begin(), availableAttributes.end(), "Liquid"), availableAttributes.end());
                        } else if (attr == "Gaseous") {
                            pixelAttributes.gaseousAttributes[pixelId] = ::Pixel::Gaseous{0.5f};
                            availableAttributes.erase(std::remove(availableAttributes.begin(), availableAttributes.end(), "Gaseous"), availableAttributes.end());
                        }
                        query.clear();
                        search[0] = '\0';
                    }
                }
            });
        });
    }

    void ImguiInterface::defaultPixelPropertiesEditor(::Pixel::DefaultPixelProperties& defaultProperties, const char* label) {
        static BarConfig sideBarConfig {
            BarOrientation::Vertical,
            "Default Pixel Properties",
            ImVec2(LAYOUT_RIGHT_W, 0.0f),
            true,
            GetDesiredPosition("right")
        };

        static Bar sideBar(sideBarConfig);

        sideBar.Draw([&]() {
            float color[3] = { defaultProperties.color.r, defaultProperties.color.g, defaultProperties.color.b };
            if (ImGui::ColorPicker3(label, color)) {
                defaultProperties.color = glm::vec3(color[0], color[1], color[2]);
            }

            if (defaultProperties.isSolid) {
                 if (ImGui::CollapsingHeader("Solid")) {
                    if (ImGui::Button("Remove Attribute")) {
                        defaultProperties.isSolid = false;
                    }
                }
            }

            if (defaultProperties.isLiquid) {
                 if (ImGui::CollapsingHeader("Liquid")) {
                    ImGui::SliderFloat("Viscosity", &defaultProperties.liquidAttributes.viscosity, 0.0f, 1.0f);
                    if (ImGui::Button("Remove Attribute")) {
                        defaultProperties.isLiquid = false;
                    }
                }
            }

            if (defaultProperties.isGaseous) {
                 if (ImGui::CollapsingHeader("Gaseous")) {
                    ImGui::SliderFloat("Density", &defaultProperties.gaseousAttributes.density, 0.0f, 1.0f);
                    if (ImGui::Button("Remove Attribute")) {
                        defaultProperties.isGaseous = false;
                    }
                }
            }

            PopupButton("Add attribute", [&]() {
                static char search[64] = "";
                ImGui::InputText("Search", search, IM_ARRAYSIZE(search));
                std::string query = search;
                std::transform(query.begin(), query.end(), query.begin(), ::tolower);

                if (!defaultProperties.isSolid) {
                    std::string attr = "Solid";
                    std::string lowerAttr = attr;
                    std::transform(lowerAttr.begin(), lowerAttr.end(), lowerAttr.begin(), ::tolower);

                    if (query.empty() || lowerAttr.find(query) != std::string::npos) {
                        if (ImGui::Selectable(attr.c_str())) {
                            defaultProperties.isSolid = true;
                            query.clear();
                            search[0] = '\0';
                        }
                    }
                }

                if (!defaultProperties.isLiquid) {
                    std::string attr = "Liquid";
                    std::string lowerAttr = attr;
                    std::transform(lowerAttr.begin(), lowerAttr.end(), lowerAttr.begin(), ::tolower);

                    if (query.empty() || lowerAttr.find(query) != std::string::npos) {
                        if (ImGui::Selectable(attr.c_str())) {
                            defaultProperties.isLiquid = true;
                            query.clear();
                            search[0] = '\0';
                        }
                    }
                }

                if (!defaultProperties.isGaseous) {
                    std::string attr = "Gaseous";
                    std::string lowerAttr = attr;
                    std::transform(lowerAttr.begin(), lowerAttr.end(), lowerAttr.begin(), ::tolower);

                    if (query.empty() || lowerAttr.find(query) != std::string::npos) {
                        if (ImGui::Selectable(attr.c_str())) {
                            defaultProperties.isGaseous = true;
                            query.clear();
                            search[0] = '\0';
                        }
                    }
                }
            });

            if (BasicButton("Reset to Defaults")) {
                defaultProperties = ::Pixel::DefaultPixelProperties();
            };
        });
    }

    void ImguiInterface::pixelSpriteHandler(bool &showDefaultPropertiesEditor, bool &isEraserActive, std::string &saveSpritePath)
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
            if (BasicButton("Pixel Parameters")) {
                showDefaultPropertiesEditor = true;
            };

            if (BasicButton("Eraser")) {
                isEraserActive = !isEraserActive;
            };

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

    glm::vec3 ImguiInterface::colorSelector(const glm::vec3& currentColor, const char* label) {
        float color[3] = { currentColor.r, currentColor.g, currentColor.b };
        if (ImGui::ColorPicker3(label, color)) {
            return glm::vec3(color[0], color[1], color[2]);
        }
        return currentColor;
    }

    namespace fs = std::filesystem;
    std::vector<std::string> spriteFiles;

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

                if (ext == ".png" || ext == ".jpg" || ext == ".dat")
                {
                    spriteFiles.push_back(p.string());
                }
            }
        }
    }

    void ImguiInterface::projectNavbar(std::string &currentSpriteFilename)
    {
        bool saveSceneRequested = false;
        bool loadSceneRequested = false;
        projectNavbar(currentSpriteFilename, saveSceneRequested, loadSceneRequested);
    }

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

    void ImguiInterface::gameObjectsBar(std::vector<::Pixel::GameObject>& gameObjects, int &selectedGameObjectIndex, engine::ComponentManager* componentManager)
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
            ImGui::TextDisabled("Pixels: %d", static_cast<int>(selected.pixelEntities.size()));

            ImGui::Separator();

            std::vector<std::string> availableComponents;

            if (!componentManager->hasComponent<ecs::components::Sprite>(selected.id)) {
                availableComponents.push_back("Sprite");
            }  else {

                auto& s = componentManager->getComponent<ecs::components::Sprite>(selected.id);


                ImGui::Checkbox("##enabledSprite", &s.enabled);
                ImGui::SameLine();
                bool open = ImGui::CollapsingHeader("Sprite Component", nullptr, ImGuiTreeNodeFlags_DefaultOpen);
                
                if (open) {
                    ImGui::BeginDisabled(!s.enabled);

                    if (ImGui::Button("Reset##Sprite"))
                    {
                        // s.texturePath = "";
                        s.width = 640.0f;
                        s.height = 640.0f;
                    }

                    static char textureBuffer[256];
                    std::strncpy(textureBuffer, s.texturePath.c_str(), sizeof(textureBuffer));
                    if (ImGui::InputText("Sprite Texture Path", textureBuffer, sizeof(textureBuffer)))
                    {
                        s.texturePath = textureBuffer;
                    }
                    ImGui::DragFloat2("Width", &s.width, 640.0f);
                    ImGui::DragFloat2("Height", &s.height, 640.0f);

                    ImGui::EndDisabled();

                    if (ImGui::Button("Remove##Sprite"))
                    {
                        componentManager->removeComponent<ecs::components::Sprite>(selected.id);
                    }
                }
            }

            if (!componentManager->hasComponent<ecs::components::Transform>(selected.id)) {
                availableComponents.push_back("Transform");
            } else {
                
                auto& t = componentManager->getComponent<ecs::components::Transform>(selected.id);
                ImGui::Checkbox("##enabledTransform", &t.enabled);
                ImGui::SameLine();
                bool open = ImGui::CollapsingHeader("Transform Component", nullptr, ImGuiTreeNodeFlags_DefaultOpen);

                if (open) {
                    ImGui::BeginDisabled(!t.enabled);

                    if (ImGui::Button("Reset##Transform"))
                    {
                        t.x =0.0f;
                        t.y = 0.0f;
                        t.rotation = 0.0f;
                        t.scaleX = 1.0f;
                        t.scaleY = 1.0f;
                    }

                    ImGui::DragFloat2("Position", &t.x, 0.1f);
                    ImGui::DragFloat("Rotation", &t.rotation, 0.1f);
                    ImGui::DragFloat2("Scale", &t.scaleX, 0.1f);

                    ImGui::EndDisabled();
            
                    if (ImGui::Button("Remove##Transform"))
                    {
                        componentManager->removeComponent<ecs::components::Transform>(selected.id);
                    }
                }
                
            }

            if (!componentManager->hasComponent<ecs::components::Velocity>(selected.id)) {
                availableComponents.push_back("Velocity");
            } else {
                auto& v = componentManager->getComponent<ecs::components::Velocity>(selected.id);

                ImGui::Checkbox("##enabledVelocity", &v.enabled);
                ImGui::SameLine();
                bool open = ImGui::CollapsingHeader("Velocity Component", nullptr, ImGuiTreeNodeFlags_DefaultOpen);

                if (open) {
                    ImGui::BeginDisabled(!v.enabled);

                    if (ImGui::Button("Reset##Velocity"))
                    {
                        v.vx = 0.0f;
                        v.vy = 0.0f;
                    }
                
                    ImGui::DragFloat2("Velocity", &v.vx, 0.1f);
                    
                    ImGui::EndDisabled();
        
                    if (ImGui::Button("Remove##Velocity"))
                    {
                        componentManager->removeComponent<ecs::components::Velocity>(selected.id);
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
                            componentManager->addComponent(selected.id, ecs::components::Sprite{});
                            availableComponents.erase(std::remove(availableComponents.begin(), availableComponents.end(), "Sprite"), availableComponents.end());
                        } else if (comp == "Transform") {
                            componentManager->addComponent(selected.id, ecs::components::Transform{});
                            availableComponents.erase(std::remove(availableComponents.begin(), availableComponents.end(), "Transform"), availableComponents.end());
                        } else if (comp == "Velocity") {
                            componentManager->addComponent(selected.id, ecs::components::Velocity{});
                            availableComponents.erase(std::remove(availableComponents.begin(), availableComponents.end(), "Velocity"), availableComponents.end());
                        }
                        query.clear();
                        search[0] = '\0';
                    }
                }
            });
        });
    }

    void ImguiInterface::drawEditorTabs(EditorMode &currentMode)
    {
        ImGuiIO &io = ImGui::GetIO();
        const float winW = io.DisplaySize.x;

        const ImVec2 tabPos(0.0f, 0.0f);
        const ImVec2 tabSize(winW, LAYOUT_TOP_H);

        constexpr ImGuiWindowFlags tabFlags =
            ImGuiWindowFlags_NoMove               |
            ImGuiWindowFlags_NoResize             |
            ImGuiWindowFlags_NoCollapse           |
            ImGuiWindowFlags_NoBringToFrontOnFocus|
            ImGuiWindowFlags_NoSavedSettings;

        // Style for tab bar
        ImGui::PushStyleColor(ImGuiCol_WindowBg,        ImVec4(0.12f, 0.12f, 0.12f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Button,          ImVec4(0.20f, 0.20f, 0.20f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   ImVec4(0.30f, 0.30f, 0.30f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,    ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Border,          ImVec4(0.05f, 0.05f, 0.05f, 1.00f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,  0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,   ImVec2(8.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(4.0f, 6.0f));

        ImGui::SetNextWindowPos(tabPos,  ImGuiCond_Always);
        ImGui::SetNextWindowSize(tabSize, ImGuiCond_Always);

        ImGui::Begin("##EditorTabs", nullptr, tabFlags);

        // Sprite Editor Tab
        bool spriteActive = (currentMode == EditorMode::SpriteEditor);
        if (spriteActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.60f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.80f));
        }

        if (ImGui::Button("Sprite Editor##Tab", ImVec2(140, 0))) {
            if (currentMode != EditorMode::SpriteEditor) {
                currentMode = EditorMode::SpriteEditor;
            }
        }

        if (spriteActive) {
            ImGui::PopStyleColor(2);
        }

        ImGui::SameLine(0.0f, 2.0f);

        // Project Editor Tab
        bool projectActive = (currentMode == EditorMode::ProjectEditor);
        if (projectActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.60f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.80f));
        }

        if (ImGui::Button("Project Editor##Tab", ImVec2(140, 0))) {
            if (currentMode != EditorMode::ProjectEditor) {
                currentMode = EditorMode::ProjectEditor;
            }
        }

        if (projectActive) {
            ImGui::PopStyleColor(2);
        }

        ImGui::End();

        ImGui::PopStyleVar(4);
        ImGui::PopStyleColor(5);
    }


} // namespace graphics