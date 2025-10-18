#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <graphics/interface/interface.hpp>
#include <graphics/renderer/renderer.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>


void showImGuiDemo() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Hello, ImGui!");
    ImGui::Text("This is a simple ImGui window.");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main() {
    graphics::Interface sdlInterface(1280, 720);
    graphics::Renderer renderer(sdlInterface.getWindow(), sdlInterface.getGLContext());

    // --- ImGui Initialization ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(sdlInterface.getWindow(), sdlInterface.getGLContext());
    ImGui_ImplOpenGL3_Init("#version 150");

    std::vector<graphics::Pixel> pixels = {
        {100, 100, 1, 0, 0, 10},
        {200, 200, 0, 1, 0, 20},
        {300, 300, 0, 0, 1, 30}
    };

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);  // let ImGui handle inputs
            if (event.type == SDL_QUIT)
                running = false;
        }

        renderer.clear();
        renderer.drawPixels(pixels);

        // Draw ImGui content
        showImGuiDemo();

        renderer.present(sdlInterface.getWindow());
    }

    // --- ImGui Cleanup ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
