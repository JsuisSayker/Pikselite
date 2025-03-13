#pragma once

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#include <graphic/GraphicEnum.hpp>

#include <imgui.h>
#include "../../extern/imgui/backends/ImGuiFileDialog.h"
#include "../../extern/imgui/backends/imgui_impl_sdl2.h"
#include "../../extern/imgui/backends/imgui_impl_sdlrenderer2.h"

#include "../../extern/icons/IconsFontAwesome5.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// rapidjson includes
#include "document.h"
#include "reader.h"
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"
#include "filereadstream.h"

#include <cstdio>
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

        void drawSprites(std::vector<Sprite> sprites, Camera camera);
        void drawPixel(Pixel pixel, Camera camera);
        void drawPixels(Camera camera);
        void drawGrid(Camera camera);
        void drawRectangle(Rectangle rectangle);

        void saveSprite(std::vector<Pixel> pixels, std::string filename);
        void createExternalAttributeFile(const std::string& filename, const std::vector<Pixel>& pixels);
        graphic::Sprite loadSpriteFromJSON(const std::string& filename);

        SDL_Renderer *getRenderer() { return _renderer; }
        SDL_Window *getWindow() { return _window; }

        void colorSelector();
        void navBar();
        void spriteEditorSidebar();
        void projectEditorSidebar();
        void pixelEditorSidebar();
        void addFileExplorer();

        void homeInterface();

        void drawInterface();

        void lightOptions();

        bool _windowOpen = true;

        std::vector<Pixel> _pixels;

        bool showHome = true;
        SpriteEditorData editorData = SpriteEditorData{Color{0, 0, 0, 255}, Color{0, 0, 0, 255}, std::variant<light, solid, liquid>{light{0, 0}}, std::variant<light, solid, liquid>{light{0, 0}}, false};
        ProjectEditorData projectData = ProjectEditorData{false, false, false, false, false, ""};
        TabSelectorData tabSelectorData = TabSelectorData{0};
        light lightData = light{0, 0};
        bool isSpriteEditor = false;
        bool isProjectEditor = false;

    private:
        SDL_Window *_window;
        SDL_Renderer *_renderer;
        ImFont* _iconFont = nullptr;
        bool colorSelectorInitialized = false;
        bool spriteEditorSidebarInitialized = false;
        bool projectEditorSidebarInitialized = false;
        bool pixelEditorSidebarInitialized = false;
        float navBarHeight = 0.0f;
    };
} // namespace sdl2