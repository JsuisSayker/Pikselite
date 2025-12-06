#include <graphics/imgui/imguiInterface.hpp>

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

            for (const auto& [id, solid] : pixelAttributes.solidAttributes) {
                if (id == pixelId) {
                     if (ImGui::CollapsingHeader("")) {
                        ImGui::TextWrapped("Solid");
                    }
                }
            }

            for (const auto& [id, liquid] : pixelAttributes.liquidAttributes) {
                if (id == pixelId) {
                     if (ImGui::CollapsingHeader("")) {
                        ImGui::TextWrapped("Liquid");
                    }
                }
            }

            for (const auto& [id, gaseous] : pixelAttributes.gaseousAttributes) {
                if (id == pixelId) {
                     if (ImGui::CollapsingHeader("")) {
                        ImGui::TextWrapped("Gaseous");
                    }
                }
            }

            PopupButton("Add attribute", [&]() {
                // Add search bar here
                // List of attributes
                // On selection: add attribute to pixel
            });
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