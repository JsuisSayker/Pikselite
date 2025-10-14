#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>

namespace graphics {
    struct Pixel {
        float x, y;
        float r, g, b;
        float size;
    };

    class Renderer {
    public:
        Renderer(SDL_Window* window, SDL_GLContext glContext);
        ~Renderer();

        void clear();
        void present(SDL_Window* window);
        void drawPixels(const std::vector<Pixel>& pixels);

    private:
        SDL_Window* _window;
        SDL_GLContext _glContext;
        GLuint _vao, _vbo, _shader;
        void initShader();
    };
}
