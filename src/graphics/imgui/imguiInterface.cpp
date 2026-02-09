#include <graphics/imgui/imguiInterface.hpp>
#include <algorithm>

namespace graphics {
    ImguiInterface::ImguiInterface(SDL_Window* window, SDL_GLContext glContext)
        : _window(window), _glContext(glContext) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
        ImGui_ImplOpenGL3_Init("#version 330 core");
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
            ImVec2(200, 400),
            true,
            ImVec2(-1, 0)
        };

        static Bar sideBar(sideBarConfig);

        sideBar.Draw([&]() {
            float color[3] = { pixel.color.r, pixel.color.g, pixel.color.b };
            if (ImGui::ColorPicker3(label, color)) {
                pixel.color = glm::vec3(color[0], color[1], color[2]);
            }

            ::Pixel::PixelEntityID pixelId = grid.getPixel(pixel.position.x, pixel.position.y);

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

    void ImguiInterface::pixelSpriteHandler()
    {
        static BarConfig sideBarConfig {
            BarOrientation::Vertical,
            "Pixel Sprite Handler",
            ImVec2(200, 400),
            true,
            ImVec2(0, 0)
        };

        static Bar sideBar(sideBarConfig);

        sideBar.Draw([&]() {
            if (BasicButton("Pixel Parameters")) {    
            };

            BasicButton("Load Sprite");
            BasicButton("Save Sprite");
        });
    }

    glm::vec3 ImguiInterface::colorSelector(const glm::vec3& currentColor, const char* label) {
        float color[3] = { currentColor.r, currentColor.g, currentColor.b };
        if (ImGui::ColorPicker3(label, color)) {
            return glm::vec3(color[0], color[1], color[2]);
        }
        return currentColor;
    }
} // namespace graphics