#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <graphics/interface/interface.hpp>
#include <graphics/renderer/renderer.hpp>

int main() {
    graphics::Interface sdlInterface(1280, 720);
    graphics::Renderer renderer(sdlInterface.getWindow(), sdlInterface.getGLContext());

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(sdlInterface.getWindow());
    }

    return 0;
}
