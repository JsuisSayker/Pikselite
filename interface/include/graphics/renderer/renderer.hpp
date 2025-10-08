#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>

namespace graphics {
    class Renderer {
        public:
            Renderer(SDL_Window* window, SDL_GLContext glContext);
            ~Renderer();

        private:
            SDL_Window* _window;
            SDL_GLContext _glContext;
    };
}
