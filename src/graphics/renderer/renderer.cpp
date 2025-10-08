#include <graphics/renderer/renderer.hpp>

namespace graphics {
    Renderer::Renderer(SDL_Window* window, SDL_GLContext glContext)
        : _window(window), _glContext(glContext)
    {
        glViewport(0, 0, 800, 600);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    }

    Renderer::~Renderer()
    {
    }
}
