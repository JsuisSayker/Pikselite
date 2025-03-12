#pragma once

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#include <graphic/GraphicEnum.hpp>


#include <imgui.h>
#include "../../extern/imgui/backends/imgui_impl_sdl2.h"
#include "../../extern/imgui/backends/imgui_impl_sdlrenderer2.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "document.h"
#include "reader.h"
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"

#include <fstream>
#include <string>
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
        void drawPixels(Camera camera);
        void drawGrid(Camera camera);
        void drawRectangle(Rectangle rectangle);

        void saveSprite(std::vector<Pixel> pixels, std::string filename);
        void createExternalAttributeFile(const std::string& filename, const std::vector<Pixel>& pixels);

        SDL_Renderer *getRenderer() { return _renderer; }
        SDL_Window *getWindow() { return _window; }

        void colorSelector();
        void navBar();
        void editorSidebar();
        void pixelSidebar();
        
        void homeInterface();

        void drawInterface();

        bool _windowOpen = true;
        
        std::vector<Pixel> _pixels;
        
        bool showHome = true;
        SpriteEditorData editorData = SpriteEditorData{false, {0, 0, 0, 255}, {0, 0, 0, 255}, false, true};
        TabSelectorData tabSelectorData = TabSelectorData{0};
        bool isSpriteEditor = false;
        bool isProjectEditor = false;

    private:
        SDL_Window *_window;
        SDL_Renderer *_renderer;
        bool colorSelectorInitialized = false;
        bool editorSidebarInitialized = false;
        bool pixelSidebarInitialized = false;
        float navBarHeight = 0.0f;
    };
} // namespace sdl2