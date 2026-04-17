/** 
 * @file imguiInterface.hpp
 * @brief Declaration of the ImGui interface for the Pixel Engine.
 * This file contains the declaration for the ImguiInterface class, which provides a wrapper around the ImGui library for use within the application.
 */

#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL_opengl.h>
#include <graphics/interface/interface.hpp>
#include <graphics/renderer/renderer.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

#include <graphics/imgui/components/components.hpp>

#include <engine/ecs/components/gameObjectComponent.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <engine/pixels/simulation/chunk.hpp>

#include <algorithm>
#include <string>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>

#include <projects.hpp> //////////////////////// temporary

namespace graphics {
    class ImguiInterface {
    public:
        ImguiInterface(SDL_Window* window, SDL_GLContext glContext);
        ~ImguiInterface();

        void showImGuiDemo();

        void pixelEditor(Pixel& pixel, ChunkGrid grid, const char* label);
        glm::vec3 colorSelector(const glm::vec3& currentColor, const char* label);

        void pixelSpriteHandler(bool &showDefaultPropertiesEditor,
                                std::string &saveSpritePath,
                                Element::ElementType& selectedElementType); // updated signature
        void spriteTopToolbar(int &selectedTool, int &brushSize, bool &isEraserActive);
        void projectTopBarEmpty();

        void defaultPixelElementEditor(Element::ElementType& elementType, const char* label);

        void projectNavbar(::std::string &currentSpriteFilename);
        void projectNavbar(::std::string &currentSpriteFilename, bool &saveSceneRequested, bool &loadSceneRequested);
        void scanSprites();

        void gameObjectsBar(std::vector<::Pixel::GameObject>& gameObjects, int &selectedGameObjectIndex);

        void fileToolBar();
        int projectOptionsBar(std::vector<projects::Project> &projects);
        int recentProjectsDisplay(std::vector<projects::Project> &projects);
        void clickableProjectOverview(projects::Project &project);

        void startFrame();
        void endFrame(SDL_Window* window);

    private:
        SDL_Window* _window;
        SDL_GLContext _glContext;
        bool newProjectCreated = false;
    };
} // namespace graphics