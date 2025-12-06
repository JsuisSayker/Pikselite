#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
// #include <SDL.h>
#include <SDL2/SDL.h> //
#include <SDL_opengl.h>
#include <graphics/interface/interface.hpp>
#include <graphics/renderer/renderer.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

#include <graphics/imgui/components/components.hpp>

namespace graphics {
    class ImguiInterface {
    public:
        ImguiInterface(SDL_Window* window, SDL_GLContext glContext);
        ~ImguiInterface();

        void showImGuiDemo();

        void pixelEditor(Pixel& pixel, const char* label);
        glm::vec3 colorSelector(const glm::vec3& currentColor, const char* label);

        void startFrame();
        void endFrame(SDL_Window* window);
    private:
        SDL_Window* _window;
        SDL_GLContext _glContext;
    };
} // namespace graphics