/**
 * @file imguiInterface.hpp
 * @brief Declaration of the ImGui interface for the Pixel Engine.
 * This file contains the declaration for the ImguiInterface class, which provides a wrapper around
 * the ImGui library for use within the application.
 */

#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL_opengl.h>
#include <algorithm>
#include <build/BuildSettings.hpp>
#include <cstring>
#include <engine/ecs/components/physicsComponent.hpp>
#include <engine/ecs/components/scriptComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/pixels/simulation/chunk.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <graphics/imgui/components/components.hpp>
#include <graphics/interface/interface.hpp>
#include <graphics/renderer/renderer.hpp>
#include <imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <projects.hpp>
#include <string>
#include <vector>

namespace graphics
{
    class ImguiInterface
    {
      public:
        ImguiInterface(SDL_Window* window, SDL_GLContext glContext);
        ~ImguiInterface();

        void showImGuiDemo();

        void      pixelEditor(Pixel& pixel, ChunkGrid grid, const char* label);
        glm::vec3 colorSelector(const glm::vec3& currentColor, const char* label);

        void pixelSpriteHandler(bool& showDefaultPropertiesEditor, std::string& saveSpritePath,
                                Element::ElementType& selectedElementType); // updated signature
        void spriteTopToolbar(int& selectedTool, int& brushSize, bool& isEraserActive);
        void projectTopBar(std::string title = "");

        void defaultPixelElementEditor(Element::ElementType& elementType, const char* label);

        void projectNavbar(::std::string& currentSpriteFilename);
        void projectNavbar(::std::string& currentSpriteFilename, bool& saveSceneRequested,
                           bool& loadSceneRequested);
        void projectNavbar(::std::string& currentSpriteFilename,
                           ::std::string& currentSceneFilename, bool& saveSceneRequested,
                           bool& loadSceneRequested);
        void projectNavbar(::std::string& currentSpriteFilename,
                           ::std::string& currentSceneFilename, bool& saveSceneRequested,
                           bool& loadSceneRequested, bool& buildGameRequested);
        void scanSprites();
        void setFileExplorerDataOnly(bool dataOnly);

        void gameObjectsBar(std::vector<::Pixel::GameObject>& gameObjects,
                            int& selectedGameObjectIndex, const projects::Project& currentProject);

        void fileToolBar();

        // Build game dialog — renders the settings modal; sets confirmed=true when user clicks
        // Build
        void buildGameSettingsDialog(BuildSettings& settings, bool& confirmed, bool& cancelled);

        // Build progress modal — call each frame while building
        void showBuildProgressModal(const char* status, float progress, bool isComplete,
                                    bool isSuccess, const char* detail = nullptr);
        int  projectOptionsBar(std::vector<projects::Project>& projects, std::string projectsPath);
        int  projectsDisplay(std::vector<projects::Project>& projects);
        void clickableProjectOverview(projects::Project& project, ImFont* nameFont = nullptr,
                                      ImFont* infoFont = nullptr);

        void startFrame();
        void endFrame(SDL_Window* window);

      private:
        struct FileEntry
        {
            std::string path;
            std::string name;
            std::string ext;
            bool        isDir = false;
        };

        void loadExplorerIcons();
        void unloadExplorerIcons();

        SDL_Window*            _window;
        SDL_GLContext          _glContext;
        float                  _uiScale          = 1.0f;
        bool                   newProjectCreated = false;
        ImFont*                fontLight         = nullptr;
        ImFont*                fontRegularSmall  = nullptr;
        ImFont*                fontRegularBig    = nullptr;
        ImFont*                fontBoldSmall     = nullptr;
        ImFont*                fontBoldBig       = nullptr;
        ImTextureID            thumbnail;
        std::string            _fileExplorerCurrentDir;
        std::vector<FileEntry> _fileExplorerEntries;
        bool                   _fileExplorerDataOnly = false;
        std::string            _fileClipboardPath;
        bool                   _fileClipboardCut = false;
        GLuint                 _iconDirTexture   = 0;
        GLuint                 _iconSceneTexture = 0;
        GLuint                 _iconDataTexture  = 0;
    };
} // namespace graphics