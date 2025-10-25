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
} // namespace graphics