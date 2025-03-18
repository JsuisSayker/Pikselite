#pragma once

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#include <graphic/GraphicEnum.hpp>

// #include <imgui.h>
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
#include <filesystem>
#include <string>
#include <iostream>
#include <stdbool.h>
#include <stdexcept>
#include <vector>
#include <magic_enum.hpp>

namespace graphic
{
    class Graphic
    {
    public:
        Graphic(bool interface = true);
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
        void createExternalAttributeFile(const std::string &filename, const std::vector<Pixel> &pixels);
        graphic::Sprite loadSpriteFromJSON(const std::string &filename, bool defaultUsage, Position actualPosition);

        SDL_Renderer *getRenderer() { return _renderer; }
        SDL_Window *getWindow() { return _window; }

        void navBar();
        void projectEditorSidebar();
        void pixelEditorSidebar();
        void spriteSelector(Camera camera);
        const char *getSpriteFileName(const std::string &path);
        void addFileExplorer();
        void addExportFileExplorer();

        void homeInterface();

        void drawInterface(Camera camera);

        void lightOptions();
        void solidOptions();
        void liquidOptions();

        bool _windowOpen = true;

        std::vector<Pixel> _pixels;

        bool showHome = true;
        SpriteEditorData editorData = SpriteEditorData{Color{0, 0, 0, 255}, Color{0, 0, 0, 255}, std::vector<std::variant<light, solid, liquid>>{light{0, 0}}, std::vector<std::variant<light, solid, liquid>>{light{0, 0}}, false};
        ProjectEditorData projectData = ProjectEditorData{false, false, false, false, false, false, false, true, "", "", "", "", ImTextureID(0)};
        TabSelectorData tabSelectorData = TabSelectorData{0};
        light lightData = light{0, 0};
        solid solidData = solid{};
        liquid liquidData = liquid{0};
        bool isSpriteEditor = false;
        bool isProjectEditor = false;
        Pixel *selectedPixel = nullptr;
        std::unordered_map<EventType, engine::Events> selectedSpriteActions = {};

    private:
        SDL_Window *_window;
        SDL_Renderer *_renderer;
        ImFont *_iconFont = nullptr;
        bool colorSelectorInitialized = false;
        bool spriteEditorSidebarInitialized = false;
        bool projectEditorSidebarInitialized = false;
        bool pixelEditorSidebarInitialized = false;
        bool spriteInputSidebarInitialized = false;
        bool showInterface;
        float navBarHeight = 0.0f;
    };
} // namespace sdl2