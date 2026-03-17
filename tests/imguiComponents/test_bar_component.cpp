#include <gtest/gtest.h>

#include <SDL2/SDL.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <GL/glew.h>

#include <graphics/imgui/components/bars.hpp>

static void InitImGuiForTests(SDL_Window*& window, SDL_GLContext& glContext) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              128, 128, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    glContext = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    glewInit();

    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");
}

static void ShutdownImGuiForTests(SDL_Window* window, SDL_GLContext glContext) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

TEST(BarDrawTests, VisibleRunsContentAndSupportsOrientation) {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    InitImGuiForTests(window, glContext);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800, 600);

    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    graphics::BarConfig cfg;
    cfg.visible = true;
    cfg.label = "test";
    cfg.size = ImVec2(200, 20);
    cfg.position = ImVec2(-1, -1);
    cfg.orientation = graphics::BarOrientation::Vertical;

    graphics::Bar bar(cfg);
    bool called = false;
    bar.Draw([&] { called = true; });
    EXPECT_TRUE(called);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ShutdownImGuiForTests(window, glContext);
}