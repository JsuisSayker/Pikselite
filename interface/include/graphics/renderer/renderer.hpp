#pragma once

#include <graphics/graphicsEnum.hpp>
#include <graphics/renderer/camera.hpp>

#include <vector>
#include <iostream>

namespace graphics {
    class Renderer {
    public:
        Renderer(SDL_Window* window, SDL_GLContext glContext);
        ~Renderer();

        void clear();
        void present(SDL_Window* window);
        void drawPixelsOverlay(const std::vector<Pixel>& pixels, float pixelSize = PIXEL_SIZE);
        void drawPixelsWCamera(const std::vector<Pixel>& pixels, const Camera2D &camera, float pixelSize = PIXEL_SIZE);

        void drawGrid(const Camera2D& camera, float cellSize, glm::vec3 color);

    private:
        SDL_Window* _window;
        SDL_GLContext _glContext;
        GLuint _vao, _vbo, _shader;
        void initShader();
    };
}
