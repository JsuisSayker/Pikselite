#pragma once

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#include <graphic/GraphicEnum.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <stdbool.h>
#include <stdexcept>
#include <vector>

namespace graphic
{
    class Graphic
    {
    public:
        Graphic();
        ~Graphic();

        void updateWindow();
        void clearWindow();

        EventType checkEvent();

        Position getPosition();

        void drawPixel(Pixel pixel, Camera camera);

        bool _windowOpen = true;
        std::vector<Pixel> _pixels;
        Camera camera = Camera{WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 4};
    protected:
    private:
        SDL_Window *_window;
        SDL_Renderer *_renderer;

    };
} // namespace sdl2