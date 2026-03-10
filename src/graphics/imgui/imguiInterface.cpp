#include <graphics/imgui/imguiInterface.hpp>
#include <algorithm>
#include <string>
#include <filesystem>
#include <vector>

namespace graphics {
    ImguiInterface::ImguiInterface(SDL_Window* window, SDL_GLContext glContext)
        : _window(window), _glContext(glContext) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
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
        ImVec2 desiredPos = GetDesiredPosition("right");
        int desiredSize = GetDesiredSize("full", BarOrientation::Vertical);

        static BarConfig sideBarConfig {
            BarOrientation::Vertical,
            "Pixel Editor",
            ImVec2(200, desiredSize),
            true,
            desiredPos
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
                    if (ImGui::Button("Remove Attribute")) {
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
                    if (ImGui::Button("Remove Attribute")) {
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
                    if (ImGui::Button("Remove Attribute")) {
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
        ImVec2 desiredPos = GetDesiredPosition("right");
        int desiredSize = GetDesiredSize("full", BarOrientation::Vertical);

        static BarConfig sideBarConfig {
            BarOrientation::Vertical,
            "Default Pixel Properties",
            ImVec2(200, desiredSize),
            true,
            desiredPos
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
        ImVec2 desiredPos = GetDesiredPosition("left");
        int desiredSize = GetDesiredSize("full", BarOrientation::Vertical);

        static BarConfig sideBarConfig {
            BarOrientation::Vertical,
            "Pixel Sprite Handler",
            ImVec2(200, desiredSize),
            true,
            desiredPos
        };

        static Bar sideBar(sideBarConfig);

        sideBar.Draw([&]() {
            if (BasicButton("Pixel Parameters")) {
                showDefaultPropertiesEditor = true;
            };

            if (BasicButton("Eraser")) {
                isEraserActive = !isEraserActive;
            };

            if (BasicButton("Load Sprite")) {
                //
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
        ImVec2 desiredPos = GetDesiredPosition("bottom");
        int desiredSize = GetDesiredSize("full", BarOrientation::Horizontal);

        static BarConfig bottomBarConfig {
            BarOrientation::Horizontal,
            "Project Navbar",
            ImVec2(desiredSize, 200),
            true,
            desiredPos
        };

        static Bar bottomBar(bottomBarConfig);

        bottomBar.Draw([&]() {

            if (BasicButton("Refresh")) {
                scanSprites();
            }

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
                ImGui::PushID(sprite.c_str());   // unique ID per sprite
            
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
            
                ImGui::PopID();   // end unique ID scope
            
                ImGui::NextColumn();
            }

            ImGui::Columns(1);
        });
    }

} // namespace graphics