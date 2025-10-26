#pragma once

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <graphics/interface/interface.hpp>
#include <graphics/renderer/renderer.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

namespace graphics {
    class ImguiInterface {
    public:
        ImguiInterface(SDL_Window* window, SDL_GLContext glContext);
        ~ImguiInterface();

        void showImGuiDemo();

        void startFrame();
        void endFrame(SDL_Window* window);
    private:
        SDL_Window* _window;
        SDL_GLContext _glContext;
    };
} // namespace graphics