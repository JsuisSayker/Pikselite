#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <graphics/interface/interface.hpp>
#include <graphics/renderer/renderer.hpp>

int main() {
    graphics::Interface sdlInterface(1280, 720);
    graphics::Renderer renderer(sdlInterface.getWindow(), sdlInterface.getGLContext());

    std::vector<graphics::Pixel> pixels;
    pixels.push_back({100, 100, 1, 0, 0, 10});
    pixels.push_back({200, 200, 0, 1, 0, 20});
    pixels.push_back({300, 300, 0, 0, 1, 30});

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            }
        }
        renderer.clear();
        renderer.drawPixels(pixels);
        renderer.present(sdlInterface.getWindow());
    }


    return 0;
}
