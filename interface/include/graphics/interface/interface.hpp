#pragma once

#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <graphics/graphicsEnum.hpp>

namespace graphics {
    class Interface {
        public:
            Interface(int width, int height);
            ~Interface();

            SDL_Window* getWindow() const { return _window; }
            SDL_GLContext getGLContext() const { return _glContext; }

            InputEventType pollEvent();
            glm::vec2 getMousePosition() const;

        private:
            SDL_Window* _window;
            SDL_GLContext _glContext;
    };
}
