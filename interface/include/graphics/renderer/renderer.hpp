#pragma once

#include <graphics/graphicsEnum.hpp>

#include <vector>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>

namespace graphics {
    class Renderer {
    public:
        Renderer(SDL_Window* window, SDL_GLContext glContext);
        ~Renderer();

        void clear();
        void present(SDL_Window* window);
        void drawPixels(const std::vector<Pixel>& pixels, float pixelSize = 1.0f);

        void drawGrid(float cellSize, float r, float g, float b);

    private:
        SDL_Window* _window;
        SDL_GLContext _glContext;
        GLuint _vao, _vbo, _shader;
        void initShader();
    };
}
