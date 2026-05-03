#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>

namespace imguiTest
{
    class ImGuiTestCommon
    {
      public:
        static void InitImGuiForTests(SDL_Window*& window, SDL_GLContext& glContext)
        {
            SDL_Init(SDL_INIT_VIDEO);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

            window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 128,
                                      128, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
            glContext = SDL_GL_CreateContext(window);

            glewExperimental = GL_TRUE;
            glewInit();

            ImGui::CreateContext();
            ImGui_ImplSDL2_InitForOpenGL(window, glContext);
            ImGui_ImplOpenGL3_Init("#version 330");
        }

        static void ShutdownImGuiForTests(SDL_Window* window, SDL_GLContext glContext)
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();

            SDL_GL_DeleteContext(glContext);
            SDL_DestroyWindow(window);
            SDL_Quit();
        }
    };
} // namespace imguiTest
