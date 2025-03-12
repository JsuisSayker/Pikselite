#pragma once

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#include <graphic/GraphicEnum.hpp>


#include <imgui.h>
#include "../../extern/imgui/backends/imgui_impl_sdl2.h"
#include "../../extern/imgui/backends/imgui_impl_sdlrenderer2.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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

        void drawPixel(Pixel pixel);
        void drawPixels();
        void drawGrid();
        void drawRectangle(Rectangle rectangle);

        void saveSprite(std::vector<Pixel> pixels, std::string filename);

        SDL_Renderer *getRenderer() { return _renderer; }
        SDL_Window *getWindow() { return _window; }

        void colorSelector();
        void navBar();
        void homeInterface();

        void drawInterface();

        bool _windowOpen = true;
        
        std::vector<Pixel> _pixels;
        Camera camera = Camera{WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 4};
        
        bool showHome = true;
        EditorData editorData = EditorData{false, {0, 0, 0, 255}, {0, 0, 0, 255}};

    private:
        SDL_Window *_window;
        SDL_Renderer *_renderer;
        bool colorSelectorInitialized = false;
    };
} // namespace sdl2