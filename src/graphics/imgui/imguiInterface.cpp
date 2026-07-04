/**
 * @file imguiInterface.cpp
 * @brief Implementation of the ImGui interface for the Pixel Engine.
 * This file contains the implementation for initializing and managing the ImGui interface within
 * the application.
 */

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <engine/scene/sceneSerializer.hpp>
#include <filesystem>
#include <fstream>
#include <graphics/imgui/components/bars.hpp>
#include <graphics/imgui/imguiInterface.hpp>
#include <nfd.hpp>
#include <stb_image.h>
#include <system_error>

namespace
{
    std::string toLowerCopy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return value;
    }

    std::filesystem::path getAssetsRoot()
    {
        return std::filesystem::absolute("assets");
    }

    std::filesystem::path makeUniqueCopyPath(const std::filesystem::path& targetDir,
                                             const std::filesystem::path& sourceName)
    {
        std::filesystem::path candidate = targetDir / sourceName;
        if (!std::filesystem::exists(candidate))
            return candidate;

        const std::string stem = sourceName.stem().string();
        const std::string ext = sourceName.extension().string();

        for (int i = 1; i < 1000; ++i)
        {
            std::filesystem::path next = targetDir / (stem + "_copy" + std::to_string(i) + ext);
            if (!std::filesystem::exists(next))
                return next;
        }

        return candidate;
    }

    GLuint loadIconTexture(const std::string& filePath)
    {
        int width = 0;
        int height = 0;
        int channels = 0;

        stbi_set_flip_vertically_on_load(false);
        unsigned char* data =
            stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!data)
            return 0;

        GLuint textureID = 0;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);

        stbi_set_flip_vertically_on_load(true);
        return textureID;
    }
} // namespace

namespace graphics
{
    /**
     * @brief Constructs an instance of the ImguiInterface.
     * @param window The SDL window to associate with the ImGui interface.
     * @param glContext The OpenGL context for rendering.
     */
    ImguiInterface::ImguiInterface(SDL_Window* window, SDL_GLContext glContext)
        : _window(window), _glContext(glContext)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // ── DPI / resolution scaling ──────────────────────────────────────────
        // Use display height as the proxy for "how big should UI be" (1080p = 1.0).
        // Clamp to a sensible band so a tiny window or a wall-sized display
        // doesn't produce unusable extremes.
        float uiScale = 1.0f;
        SDL_DisplayMode dm{};
        const int displayIndex = (window != nullptr) ? SDL_GetWindowDisplayIndex(window) : 0;
        if (SDL_GetCurrentDisplayMode(displayIndex >= 0 ? displayIndex : 0, &dm) == 0 && dm.h > 0)
        {
            uiScale = std::clamp(static_cast<float>(dm.h) / 1080.0f, 1.0f, 3.0f);
        }
        _uiScale = uiScale;

        // Scale shared layout constants used by the toolbars/sidebars so they
        // track font/UI sizing on high-DPI screens.
        LAYOUT_TOP_H = 40.0f * uiScale;
        LAYOUT_BOTTOM_H = 180.0f * uiScale;
        LAYOUT_LEFT_W = 220.0f * uiScale;
        LAYOUT_RIGHT_W = 260.0f * uiScale;

        ImGuiIO& io = ImGui::GetIO();
        auto scaledPx = [uiScale](float baseSize)
        { return std::max(1.0f, std::floor(baseSize * uiScale)); };
        fontLight =
            io.Fonts->AddFontFromFileTTF("assets/fonts/InriaSans-Light.ttf", scaledPx(13.0f));
        fontRegularSmall =
            io.Fonts->AddFontFromFileTTF("assets/fonts/InriaSans-Regular.ttf", scaledPx(15.0f));
        fontRegularBig =
            io.Fonts->AddFontFromFileTTF("assets/fonts/InriaSans-Regular.ttf", scaledPx(28.0f));
        fontBoldSmall =
            io.Fonts->AddFontFromFileTTF("assets/fonts/InriaSans-Bold.ttf", scaledPx(25.0f));
        fontBoldBig =
            io.Fonts->AddFontFromFileTTF("assets/fonts/InriaSans-Bold.ttf", scaledPx(50.0f));

        ImGui::StyleColorsDark();

        // ── Global Unity-like style overrides ─────────────────────────────────
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.ChildRounding = 4.0f;
        style.FrameRounding = 3.0f;
        style.GrabRounding = 3.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 3.0f;
        style.TabRounding = 4.0f;
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.ItemSpacing = ImVec2(6.0f, 5.0f);
        style.ScrollbarSize = 12.0f;
        style.GrabMinSize = 8.0f;

        ImVec4* c = style.Colors;
        c[ImGuiCol_Text] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        c[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
        c[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        c[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.98f);
        c[ImGuiCol_Border] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        c[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        c[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        c[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        c[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        c[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        c[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        c[ImGuiCol_Button] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        c[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.50f);
        c[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.85f);
        c[ImGuiCol_Separator] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        c[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        c[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        c[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.50f);
        c[ImGuiCol_TabActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        c[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        c[ImGuiCol_PlotHistogram] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);

        // Scale paddings/spacing/rounding/etc. by the same UI scale we used for fonts.
        // Done last so it applies uniformly to all values configured above.
        style.ScaleAllSizes(uiScale);

        ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        _fileExplorerCurrentDir = getAssetsRoot().generic_string();
        loadExplorerIcons();
    }

    /**
     * @brief Destructor for the ImguiInterface class. Shuts down the ImGui context and cleans up
     * resources.
     */
    ImguiInterface::~ImguiInterface()
    {
        unloadExplorerIcons();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void ImguiInterface::loadExplorerIcons()
    {
        if (_iconDirTexture || _iconSceneTexture || _iconDataTexture)
            return;

        _iconDirTexture = loadIconTexture("assets/ui/icons/dir.png");
        _iconSceneTexture = loadIconTexture("assets/ui/icons/scene.png");
        _iconDataTexture = loadIconTexture("assets/ui/icons/sprite.png");
    }

    void ImguiInterface::unloadExplorerIcons()
    {
        if (_iconDirTexture)
        {
            glDeleteTextures(1, &_iconDirTexture);
            _iconDirTexture = 0;
        }
        if (_iconSceneTexture)
        {
            glDeleteTextures(1, &_iconSceneTexture);
            _iconSceneTexture = 0;
        }
        if (_iconDataTexture)
        {
            glDeleteTextures(1, &_iconDataTexture);
            _iconDataTexture = 0;
        }
    }

    /**
     * @brief Starts a new ImGui frame. This should be called at the beginning of each frame before
     * any ImGui rendering calls.
     */
    void ImguiInterface::startFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    /**
     * @brief Ends the current ImGui frame and renders the ImGui draw data. This should be called at
     * the end of each frame after all ImGui rendering calls.
     * @param window The SDL window to render to.
     */
    void ImguiInterface::endFrame(SDL_Window* window)
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImguiInterface::setFileExplorerDataOnly(bool dataOnly)
    {
        _fileExplorerDataOnly = dataOnly;
    }

    /**
     * @brief Displays the ImGui demo window. This is a built-in ImGui feature that showcases
     * various ImGui components and their usage.
     */
    void ImguiInterface::showImGuiDemo()
    {
        ImGui::Begin("Hello, ImGui!");
        ImGui::Text("This is a simple ImGui window.");
        ImGui::End();
    }

    /**
     * @brief Displays the pixel editor sidebar for a given pixel, allowing users to modify its
     color and attributes. The sidebar includes options for adding or removing solid, liquid, and
     gaseous attributes, as well as a color picker for changing the pixel's color.
     * @param pixel The pixel to be edited.
     * @param grid The chunk grid containing the pixel.
     * @param pixelAttributes The attributes of the pixel.
     * @param label The label for the color picker.

     */
    void ImguiInterface::pixelEditor(Pixel& pixel, ChunkGrid grid, const char* label)
    {
        // TODO
    }

    /**
     * @brief Displays the default pixel properties editor in a sidebar, allowing users to modify
     * the default color and attributes for new pixels. The editor includes options for adding or
     * removing solid, liquid, and gaseous attributes, as well as a color picker for changing the
     * default pixel color.
     * @param defaultProperties The default pixel properties to be edited.
     * @param label The label for the color picker.
     */
    void ImguiInterface::defaultPixelElementEditor(Element::ElementType& elementType,
                                                   const char* label)
    {
        const char* comboLabel = (label && label[0] != '\0') ? label : "Element Type";
        int current = static_cast<int>(elementType);

        auto currentName = [&]() -> const char*
        {
            if (current >= 0 && current < 256 && !g_elements[current].name.empty())
            {
                return g_elements[current].name.c_str();
            }
            return "Unknown";
        };

        if (ImGui::BeginCombo(comboLabel, currentName()))
        {
            for (int i = 0; i < 256; ++i)
            {
                if (g_elements[i].name.empty())
                    continue; // only show registered elements

                const bool isSelected = (i == current);
                if (ImGui::Selectable(g_elements[i].name.c_str(), isSelected))
                {
                    current = i;
                    elementType = static_cast<Element::ElementType>(i);
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    /**
     * @brief Displays the pixel sprite handler sidebar, which provides options for editing pixel
     * parameters and saving the current sprite. The sidebar includes a popup for entering the name
     * of the sprite when saving.
     * @param showDefaultPropertiesEditor A reference to a boolean that indicates whether to show
     * the default properties editor.
     * @param saveSpritePath A reference to a string that will hold the path where the sprite should
     * be saved.
     */
    void ImguiInterface::pixelSpriteHandler(bool&                 showDefaultPropertiesEditor,
                                            std::string&          saveSpritePath,
                                            Element::ElementType& selectedElementType, std::string currentProjectAssetsPath)
    {
        static BarConfig sideBarConfig{BarOrientation::Vertical, "Pixel Sprite Handler",
                                       ImVec2(LAYOUT_LEFT_W, 0.0f), true,
                                       GetDesiredPosition("left")};

        static Bar sideBar(sideBarConfig);

        sideBar.Draw(
            [&]()
            {
                // Element dropdown directly in handler
                defaultPixelElementEditor(selectedElementType, "Element Type");

                if (BasicButton("Save Sprite"))
                {
                    ImGui::OpenPopup("NameNewSpritePopup");
                };

                static char spriteName[128] = "";

                if (ImGui::BeginPopupModal("NameNewSpritePopup", NULL,
                                           ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Enter sprite name:");
                    ImGui::InputText("Name##SpriteInput", spriteName, sizeof(spriteName));
                    ImGui::Spacing();

                    if (ImGui::Button("Save##SpriteButton", ImVec2(120, 0)))
                    {
                        std::string path = currentProjectAssetsPath + "/" + std::string(spriteName) + ".dat";
                        saveSpritePath   = path;
                        spriteName[0]    = '\0';
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel##SpriteButton", ImVec2(120, 0)))
                    {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            });
    }

    /**
     * @brief Displays the top toolbar for the sprite editor, which includes options for selecting
     * tools (paint or eraser) and adjusting brush size. The toolbar also manages the state of the
     * selected tool and whether the eraser is active.
     * @param selectedTool A reference to an integer that indicates the currently selected tool (0
     * for paint, 1 for eraser).
     * @param brushSize A reference to an integer that represents the current brush size.
     * @param isEraserActive A reference to a boolean that indicates whether the eraser tool is
     * currently active.
     */
    void ImguiInterface::spriteTopToolbar(int& selectedTool, int& brushSize, bool& isEraserActive,
                                          bool& clearAllRequested)
    {
        static BarConfig topBarConfig{BarOrientation::Horizontal, "Sprite Tools",
                                      ImVec2(0.0f, LAYOUT_TOP_H), true, GetDesiredPosition("top")};

        static Bar topBar(topBarConfig);

        topBar.Draw(
            [&]()
            {
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Tool:");
                ImGui::SameLine();

                if (ImGui::Selectable("Paint", selectedTool == 0, 0, ImVec2(80, 0)))
                {
                    selectedTool = 0;
                    isEraserActive = false;
                }
                ImGui::SameLine();
                if (ImGui::Selectable("Eraser", selectedTool == 1, 0, ImVec2(80, 0)))
                {
                    selectedTool = 1;
                    isEraserActive = true;
                }

                ImGui::SameLine();
                ImGui::TextUnformatted("|");
                ImGui::SameLine();

                ImGui::SetNextItemWidth(180.0f);
                ImGui::SliderInt("Brush Size", &brushSize, 1, 8);

                ImGui::SameLine();
                ImGui::TextUnformatted("|");
                ImGui::SameLine();

                if (ImGui::Button("Clear All"))
                {
                    clearAllRequested = true;
                }

                if (selectedTool == 1)
                {
                    isEraserActive = true;
                }
            });
    }

    /**
     * @brief Displays an empty top toolbar for the project editor.
     */
    void ImguiInterface::projectTopBar(bool& saveSceneRequested, std::string title)
    {
        static BarConfig topBarConfig{BarOrientation::Horizontal, "Project Top Bar",
                                      ImVec2(0.0f, LAYOUT_TOP_H), true, GetDesiredPosition("top")};

        static Bar topBar(topBarConfig);
        topBar.Draw(
            [&]()
            {
                if (BasicButton("Save"))
                {
                    saveSceneRequested = true;
                }

                ImVec2 textSize = ImGui::CalcTextSize(title.c_str());
                ImVec2 windowSize = ImGui::GetWindowSize();

                ImGui::SetCursorPos(
                    ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f));

                ImGui::Text("%s", title.c_str());
            });
    }

    /**
     * @brief Displays a color selector widget in the ImGui interface, allowing users to pick a
     * color. The function takes the current color as input and returns the selected color if it has
     * been changed, or the original color if no change was made.
     * @param currentColor The current color to be displayed in the color picker.
     * @param label The label for the color picker widget.
     * @return The new color selected by the user, or the original color if no change was made.
     */
    glm::vec3 ImguiInterface::colorSelector(const glm::vec3& currentColor, const char* label)
    {
        float color[3] = {currentColor.r, currentColor.g, currentColor.b};
        if (ImGui::ColorPicker3(label, color))
        {
            return glm::vec3(color[0], color[1], color[2]);
        }
        return currentColor;
    }

    namespace fs = std::filesystem;

    /**
     * @brief Scans the current assets directory for files and subfolders.
     */
    void ImguiInterface::scanSprites(std::filesystem::path folderPath)
    {
        _fileExplorerEntries.clear();

        if (!fs::exists(folderPath) || !fs::is_directory(folderPath))
            return;

        if (_fileExplorerCurrentDir.empty())
            _fileExplorerCurrentDir = folderPath.generic_string();

        std::error_code ec;
        fs::path currentPath = fs::weakly_canonical(fs::path(_fileExplorerCurrentDir), ec);
        fs::path canonicalRoot = fs::weakly_canonical(folderPath, ec);

        if (ec || currentPath.empty() ||
            currentPath.generic_string().find(canonicalRoot.generic_string()) != 0)
        {
            currentPath = canonicalRoot;
        }

        if (!fs::exists(currentPath))
            currentPath = canonicalRoot;

        _fileExplorerCurrentDir = currentPath.generic_string();

        for (const auto& entry : fs::directory_iterator(currentPath))
        {
            FileEntry item;
            item.path = entry.path().string();
            item.name = entry.path().filename().string();
            item.isDir = entry.is_directory();
            item.ext = toLowerCopy(entry.path().extension().string());
            _fileExplorerEntries.push_back(std::move(item));
        }
    }

    /**
     * @brief Displays the project navbar, which includes a refresh button to scan for new sprites,
     * buttons to save and load scenes, and a grid of available sprite thumbnails. When a sprite
     * thumbnail is clicked, the current sprite filename is updated to reflect the selected sprite.
     * @param currentSpriteFilename A reference to a string that will hold the filename of the
     * currently selected sprite.
     */
    void ImguiInterface::projectNavbar(std::string& currentSpriteFilename, std::filesystem::path currentProjectAssetsPath, std::filesystem::path currentProjectScenesPath)
    {
        bool saveSceneRequested = false;
        bool loadSceneRequested = false;
        bool dummyBuildRequest = false;
        static std::string dummySceneFilename;
        projectAssetsNavbar(currentSpriteFilename, dummySceneFilename, saveSceneRequested, loadSceneRequested, dummyBuildRequest, currentProjectAssetsPath, currentProjectScenesPath);
    }

    void ImguiInterface::projectAssetsNavbar(std::string& currentSpriteFilename,
                                       std::string& currentSceneFilename, bool& saveSceneRequested,
                                       bool& loadSceneRequested, bool& buildGameRequested, std::filesystem::path currentProjectAssetsPath, std::filesystem::path currentProjectScenesPath) ////////////////////////////////////////////////////////
    {
        static BarConfig bottomBarConfig{BarOrientation::Horizontal, "Project Navbar",
                                         ImVec2(0.0f, LAYOUT_BOTTOM_H), true,
                                         GetDesiredPosition("bottom")};

        static Bar bottomBar(bottomBarConfig);

        bottomBar.Draw(
            [&]()
            {
                const fs::path rootPath = getAssetsRoot();
                if (_fileExplorerEntries.empty())
                    scanSprites(currentProjectAssetsPath);

                bool canGoUp = fs::path(_fileExplorerCurrentDir) != rootPath;
                if (!canGoUp)
                    ImGui::BeginDisabled();
                if (BasicButton("Up"))
                {
                    fs::path parent = fs::path(_fileExplorerCurrentDir).parent_path();
                    if (!parent.empty())
                    {
                        _fileExplorerCurrentDir = parent.generic_string();
                        scanSprites(currentProjectAssetsPath);
                    }
                }
                if (!canGoUp)
                    ImGui::EndDisabled();

                ImGui::SameLine();
                if (BasicButton("Refresh"))
                    scanSprites(currentProjectAssetsPath);

                ImGui::SameLine();
                if (BasicButton("New File"))
                    ImGui::OpenPopup("NewFilePopup");

                // "Save Scene" lives on the top-left toolbar now — projectTopBar drives
                // `saveSceneRequested`. The bottom bar keeps file-explorer actions only.

                ImGui::SameLine();
                if (BasicButton("Build Game"))
                    buildGameRequested = true;

                const std::string relativeDir =
                    fs::relative(_fileExplorerCurrentDir, rootPath).generic_string();
                ImGui::SameLine();
                ImGui::Text("Dir: %s", relativeDir == "." ? "" : relativeDir.c_str());

                ImGui::Separator();

                static char renameBuffer[128] = "";
                static std::string renameTargetPath;
                static bool openRenamePopup = false;

                static char newFileBuffer[128] = "";
                static char newSceneBuffer[128] = "";
                static bool openCreateScenePopup = false;

                auto pasteIntoDirectory = [&](const fs::path& targetDir)
                {
                    if (_fileClipboardPath.empty() || !fs::exists(_fileClipboardPath))
                        return;

                    const fs::path sourcePath = fs::path(_fileClipboardPath);
                    const fs::path sourceName = sourcePath.filename();
                    const fs::path targetPath = makeUniqueCopyPath(targetDir, sourceName);

                    std::error_code ec;
                    if (fs::is_directory(sourcePath))
                    {
                        fs::path canonicalSource = fs::weakly_canonical(sourcePath, ec);
                        fs::path canonicalTarget = fs::weakly_canonical(targetDir, ec);
                        if (!ec && canonicalTarget.string().find(canonicalSource.string()) == 0)
                            return;

                        fs::copy(sourcePath, targetPath,
                                 fs::copy_options::recursive | fs::copy_options::overwrite_existing,
                                 ec);
                    }
                    else
                    {
                        fs::copy_file(sourcePath, targetPath, fs::copy_options::overwrite_existing,
                                      ec);
                    }

                    if (!ec && _fileClipboardCut)
                    {
                        if (fs::is_directory(sourcePath))
                            fs::remove_all(sourcePath, ec);
                        else
                            fs::remove(sourcePath, ec);
                        _fileClipboardPath.clear();
                        _fileClipboardCut = false;
                    }

                    scanSprites(currentProjectAssetsPath);
                };

                if (ImGui::BeginPopupContextWindow("ProjectNavbarContext",
                                                   ImGuiPopupFlags_MouseButtonRight |
                                                       ImGuiPopupFlags_NoOpenOverItems))
                {
                    if (ImGui::MenuItem("Create Scene"))
                    {
                        openCreateScenePopup = true;
                    }

                    const bool hasClipboard = !_fileClipboardPath.empty();
                    if (ImGui::MenuItem("Paste", NULL, false, hasClipboard))
                    {
                        pasteIntoDirectory(fs::path(_fileExplorerCurrentDir));
                    }
                    ImGui::EndPopup();
                }

                if (openCreateScenePopup)
                {
                    ImGui::OpenPopup("CreateScenePopup");
                    openCreateScenePopup = false;
                }

                bool createScenePopupOpen = true;
                if (ImGui::BeginPopupModal("CreateScenePopup", &createScenePopupOpen,
                                           ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Create new scene in current folder");
                    ImGui::InputText("Name", newSceneBuffer, sizeof(newSceneBuffer));

                    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                    {
                        newSceneBuffer[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::Button("Create"))
                    {
                        if (newSceneBuffer[0] != '\0')
                        {
                            fs::path newPath = fs::path(_fileExplorerCurrentDir) / newSceneBuffer;
                            if (newPath.extension().empty())
                                newPath.replace_extension(".scene");

                            engine::scene::SceneData emptyScene;
                            emptyScene.nextGameObjectId = 1;
                            engine::scene::saveSceneToFile(newPath.string(), emptyScene);

                            currentSceneFilename = newPath.string();
                            currentSpriteFilename.clear();
                            loadSceneRequested = true;

                            newSceneBuffer[0] = '\0';
                            scanSprites(currentProjectAssetsPath);
                        }
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel"))
                    {
                        newSceneBuffer[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                if (!createScenePopupOpen)
                {
                    newSceneBuffer[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::BeginPopupModal("NewFilePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Create file in current folder");
                    ImGui::InputText("Name", newFileBuffer, sizeof(newFileBuffer));

                    if (ImGui::Button("Create"))
                    {
                        if (newFileBuffer[0] != '\0')
                        {
                            fs::path newPath = fs::path(_fileExplorerCurrentDir) / newFileBuffer;
                            std::ofstream outFile(newPath.string());
                            outFile.close();
                            newFileBuffer[0] = '\0';
                            scanSprites(currentProjectAssetsPath);
                        }
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel"))
                    {
                        newFileBuffer[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                float thumbnailSize = 65.0f;
                float padding = 15.0f;
                float cellSize = thumbnailSize + padding;

                float panelWidth = ImGui::GetContentRegionAvail().x;

                int columns = (int)(panelWidth / cellSize);
                if (columns < 1)
                    columns = 1;

                std::sort(_fileExplorerEntries.begin(), _fileExplorerEntries.end(),
                          [](const FileEntry& a, const FileEntry& b)
                          {
                              if (a.isDir != b.isDir)
                                  return a.isDir > b.isDir;
                              return a.name < b.name;
                          });

                ImGui::Columns(columns, 0, false);

                for (const auto& entry : _fileExplorerEntries)
                {
                    if (_fileExplorerDataOnly && !entry.isDir && entry.ext != ".dat")
                    {
                        continue;
                    }

                    ImGui::PushID(entry.path.c_str());

                    ImGui::BeginGroup();

                    float columnWidth = ImGui::GetColumnWidth();
                    float offset = (columnWidth - thumbnailSize) * 0.5f;

                    if (offset > 0)
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

                    ImGui::InvisibleButton("##icon", ImVec2(thumbnailSize, thumbnailSize));
                    const bool clicked = ImGui::IsItemClicked();
                    const bool doubleClicked = ImGui::IsItemHovered() &&
                                               ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    const ImVec2 rectMin = ImGui::GetItemRectMin();
                    const ImVec2 rectMax = ImGui::GetItemRectMax();
                    const ImU32 bgColor = ImGui::GetColorU32(
                        ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
                    const ImU32 lineColor = ImGui::GetColorU32(ImGuiCol_Text);
                    const ImU32 accentColor = ImGui::GetColorU32(ImGuiCol_ButtonActive);

                    drawList->AddRectFilled(rectMin, rectMax, bgColor, 4.0f);

                    const float iconPad = 12.0f;
                    const ImVec2 iconMin(rectMin.x + iconPad, rectMin.y + iconPad);
                    const ImVec2 iconMax(rectMax.x - iconPad, rectMax.y - iconPad);

                    const bool isScene = entry.ext == ".scene";
                    const bool isData = entry.ext == ".dat" || entry.ext == ".data";

                    GLuint iconTexture = 0;
                    if (entry.isDir)
                        iconTexture = _iconDirTexture;
                    else if (isScene)
                        iconTexture = _iconSceneTexture;
                    else if (isData)
                        iconTexture = _iconDataTexture;

                    if (iconTexture != 0)
                    {
                        drawList->AddImage(static_cast<ImTextureID>(iconTexture), iconMin, iconMax);
                    }
                    else
                    {
                        drawList->AddRect(iconMin, iconMax, lineColor, 3.0f, 0, 1.5f);
                    }

                    if (ImGui::BeginPopupContextItem("FileContext"))
                    {
                        if (ImGui::MenuItem("Rename"))
                        {
                            strncpy(renameBuffer, entry.name.c_str(), sizeof(renameBuffer));
                            renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                            renameTargetPath = entry.path;
                            openRenamePopup = true;
                        }

                        if (ImGui::MenuItem("Copy"))
                        {
                            _fileClipboardPath = entry.path;
                            _fileClipboardCut = false;
                        }

                        if (ImGui::MenuItem("Cut"))
                        {
                            _fileClipboardPath = entry.path;
                            _fileClipboardCut = true;
                        }

                        if (entry.isDir)
                        {
                            const bool hasClipboard = !_fileClipboardPath.empty();
                            if (ImGui::MenuItem("Paste Into", NULL, false, hasClipboard))
                            {
                                pasteIntoDirectory(fs::path(entry.path));
                            }
                        }
                        ImGui::EndPopup();
                    }

                    if (entry.isDir)
                    {
                        if (doubleClicked)
                        {
                            _fileExplorerCurrentDir = fs::path(entry.path).generic_string();
                            scanSprites(currentProjectAssetsPath);
                        }
                    }
                    else
                    {
                        if (clicked)
                        {
                            if (!_fileExplorerDataOnly && entry.ext == ".scene")
                            {
                                currentSceneFilename = entry.path;
                                currentSpriteFilename.clear();
                            }
                            else
                            {
                                currentSpriteFilename = entry.path;
                            }
                        }

                        if (!_fileExplorerDataOnly && doubleClicked && entry.ext == ".scene")
                        {
                            currentSceneFilename = entry.path;
                            loadSceneRequested = true;
                            currentSpriteFilename.clear();
                        }
                    }

                    float textWidth = ImGui::CalcTextSize(entry.name.c_str()).x;
                    float textOffset = (columnWidth - textWidth) * 0.5f;

                    if (textOffset > 0)
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);

                    ImGui::TextWrapped("%s", entry.name.c_str());

                    ImGui::EndGroup();

                    ImGui::PopID();

                    ImGui::NextColumn();
                }

                ImGui::Columns(1);

                if (openRenamePopup)
                {
                    ImGui::OpenPopup("RenameFilePopup");
                    openRenamePopup = false;
                }

                if (ImGui::BeginPopupModal("RenameFilePopup", NULL,
                                           ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Rename file or folder");
                    ImGui::InputText("New Name", renameBuffer, sizeof(renameBuffer));

                    if (ImGui::Button("Save"))
                    {
                        if (!renameTargetPath.empty() && renameBuffer[0] != '\0')
                        {
                            fs::path oldPath = renameTargetPath;
                            fs::path newPath = oldPath.parent_path() / renameBuffer;
                            std::error_code renameError;
                            fs::rename(oldPath, newPath, renameError);
                            scanSprites(currentProjectAssetsPath);
                        }
                        renameBuffer[0] = '\0';
                        renameTargetPath.clear();
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel"))
                    {
                        renameBuffer[0] = '\0';
                        renameTargetPath.clear();
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            });
    }

    void ImguiInterface::filterSprites(std::vector<FileEntry>& sprites)
    {
        for (const auto& entry : _fileExplorerEntries)
        {
            if (!entry.isDir && (entry.ext == ".dat" || entry.ext == ".data"))
            {
                sprites.push_back(entry);
            }
        }
    }
    void ImguiInterface::spriteAssetsNavbar(::std::string& currentSpriteFilename, std::filesystem::path currentProjectAssetsPath)
    {
        static BarConfig bottomBarConfig{BarOrientation::Horizontal, "Project Navbar",
                                         ImVec2(0.0f, LAYOUT_BOTTOM_H), true,
                                         GetDesiredPosition("bottom")};

        static Bar bottomBar(bottomBarConfig);

        bottomBar.Draw(
            [&]()
            {
                std::vector<FileEntry> sprites;
                
                static char renameBuffer[128] = "";
                static std::string renameTargetPath;
                static bool openRenamePopup = false;

                float thumbnailSize = 65.0f;
                float padding = 15.0f;
                float cellSize = thumbnailSize + padding;

                float panelWidth = ImGui::GetContentRegionAvail().x;

                int columns = (int)(panelWidth / cellSize);
                if (columns < 1)
                    columns = 1;

                ImGui::Columns(columns, 0, false);

                scanSprites(currentProjectAssetsPath);

                if (_fileExplorerEntries.empty())
                {
                    ImGui::Text("No sprites found in the current directory.");
                } 
                else
                {
                    filterSprites(sprites);

                    for (const auto& entry : sprites)
                    {
                        ImGui::PushID(entry.path.c_str());

                        ImGui::BeginGroup();

                        float columnWidth = ImGui::GetColumnWidth();
                        float offset = (columnWidth - thumbnailSize) * 0.5f;

                        if (offset > 0)
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

                        ImGui::InvisibleButton("##icon", ImVec2(thumbnailSize, thumbnailSize));
                        const bool clicked = ImGui::IsItemClicked();
                        const bool doubleClicked = ImGui::IsItemHovered() &&
                                                   ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

                        ImDrawList* drawList = ImGui::GetWindowDrawList();
                        const ImVec2 rectMin = ImGui::GetItemRectMin();
                        const ImVec2 rectMax = ImGui::GetItemRectMax();
                        const ImU32 bgColor = ImGui::GetColorU32(
                            ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
                        const ImU32 lineColor = ImGui::GetColorU32(ImGuiCol_Text);
                        const ImU32 accentColor = ImGui::GetColorU32(ImGuiCol_ButtonActive);

                        drawList->AddRectFilled(rectMin, rectMax, bgColor, 4.0f);

                        const float iconPad = 12.0f;
                        const ImVec2 iconMin(rectMin.x + iconPad, rectMin.y + iconPad);
                        const ImVec2 iconMax(rectMax.x - iconPad, rectMax.y - iconPad);

                        GLuint iconTexture = 0;

                        iconTexture = _iconDataTexture;

                        if (iconTexture != 0)
                        {
                            drawList->AddImage(static_cast<ImTextureID>(iconTexture), iconMin, iconMax);
                        }
                        else
                        {
                            drawList->AddRect(iconMin, iconMax, lineColor, 3.0f, 0, 1.5f);
                        }

                        if (ImGui::BeginPopupContextItem("FileContext"))
                        {
                            if (ImGui::MenuItem("Rename"))
                            {
                                strncpy(renameBuffer, entry.name.c_str(), sizeof(renameBuffer));
                                renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                                renameTargetPath = entry.path;
                                openRenamePopup = true;
                            }

                            if (ImGui::MenuItem("Copy"))
                            {
                                _fileClipboardPath = entry.path;
                                _fileClipboardCut = false;
                            }

                            if (ImGui::MenuItem("Cut"))
                            {
                                _fileClipboardPath = entry.path;
                                _fileClipboardCut = true;
                            }

                            ImGui::EndPopup();
                        }

                        if (clicked)
                        {
                            currentSpriteFilename = entry.path;       
                        }

                        float textWidth = ImGui::CalcTextSize(entry.name.c_str()).x;
                        float textOffset = (columnWidth - textWidth) * 0.5f;

                        if (textOffset > 0)
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);

                        ImGui::TextWrapped("%s", entry.name.c_str());

                        ImGui::EndGroup();

                        ImGui::PopID();

                        ImGui::NextColumn();
                    }

                    ImGui::Columns(1);

                    if (openRenamePopup)
                    {
                        ImGui::OpenPopup("RenameFilePopup");
                        openRenamePopup = false;
                    }

                    if (ImGui::BeginPopupModal("RenameFilePopup", NULL,
                                               ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        ImGui::Text("Rename file or folder");
                        ImGui::InputText("New Name", renameBuffer, sizeof(renameBuffer));

                        if (ImGui::Button("Save"))
                        {
                            if (!renameTargetPath.empty() && renameBuffer[0] != '\0')
                            {
                                fs::path oldPath = renameTargetPath;
                                fs::path newPath = oldPath.parent_path() / renameBuffer;
                                std::error_code renameError;
                                fs::rename(oldPath, newPath, renameError);
                                scanSprites(currentProjectAssetsPath);

                            }
                            renameBuffer[0] = '\0';
                            renameTargetPath.clear();
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Cancel"))
                        {
                            renameBuffer[0] = '\0';
                            renameTargetPath.clear();
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndPopup();
                    }
                }
            }
        );
    }

    /**
     * @brief Displays the game objects bar, which consists of a hierarchy view of all game objects
     * and an inspector for the selected game object. The hierarchy allows users to select, rename,
     * and delete game objects, while the inspector displays properties of the selected game object
     * and allows users to edit them.
     * @param gameObjects A reference to a vector of game objects to be displayed in the hierarchy.
     * @param selectedGameObjectIndex A reference to an integer that indicates the index of the
     * currently selected game object in the hierarchy.
     * @param currentProject A reference to the current project.
     */
    void ImguiInterface::gameObjectsBar(std::vector<::Pixel::GameObject>& gameObjects,
                                        int& selectedGameObjectIndex,
                                        const projects::Project& currentProject,
                                        int& deleteRequestIndex)
    {
        static BarConfig sideBarConfig{BarOrientation::Vertical, "Hierarchy",
                                       ImVec2(LAYOUT_LEFT_W, 0.0f), true,
                                       GetDesiredPosition("left")};

        static Bar sideBar(sideBarConfig);

        sideBar.Draw(
            [&]()
            {
                static char searchBuffer[128] = "";
                static char renameBuffer[128] = "";
                static int renameIndex = -1;
                static bool openRenamePopup = false;
                static bool focusRename = true;

                ImGui::InputTextWithHint("##SearchObjects", "Search objects...", searchBuffer,
                                         sizeof(searchBuffer));

                std::string lowerSearch = searchBuffer;
                std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(),
                               ::tolower);

                ImGui::BeginChild("GameObjectList", ImVec2(0, 0), true);
                for (size_t i = 0; i < gameObjects.size(); ++i)
                {
                    const std::string objName =
                        gameObjects[i].name.empty()
                            ? ("GameObject " + std::to_string(gameObjects[i].id))
                            : gameObjects[i].name;

                    if (!lowerSearch.empty())
                    {
                        std::string lowerName = objName;
                        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                                       ::tolower);

                        if (lowerName.find(lowerSearch) == std::string::npos)
                            continue;
                    }

                    ImGui::PushID((int)i);

                    if (ImGui::Selectable(objName.c_str(), selectedGameObjectIndex == (int)i))
                        selectedGameObjectIndex = (int)i;

                    // If right-clicked
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::MenuItem("Rename"))
                        {
                            strncpy(renameBuffer, objName.c_str(), sizeof(renameBuffer));
                            renameBuffer[sizeof(renameBuffer) - 1] = '\0';

                            renameIndex = (int)i;
                            openRenamePopup = true;
                        }

                        if (ImGui::MenuItem("Delete"))
                        {
                            // Defer the actual deletion — the caller has the chunk grid /
                            // ECS context needed to fully clean up the GameObject's pixels.
                            deleteRequestIndex = (int)i;

                            ImGui::EndPopup();
                            ImGui::PopID();
                            break;
                        }

                        ImGui::EndPopup();
                    }

                    ImGui::PopID();
                }
                ImGui::EndChild();

                if (openRenamePopup)
                {
                    ImGui::OpenPopup("RenameObjectPopup");
                    openRenamePopup = false;
                }

                if (ImGui::BeginPopupModal("RenameObjectPopup", NULL,
                                           ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Rename Game Object");

                    if (focusRename)
                    {
                        ImGui::SetKeyboardFocusHere();
                        focusRename = false;
                    }

                    ImGui::InputText("New Name", renameBuffer, sizeof(renameBuffer));

                    if (ImGui::Button("Save"))
                    {
                        if (renameIndex >= 0 && renameIndex < (int)gameObjects.size())
                            gameObjects[renameIndex].name = renameBuffer;

                        renameBuffer[0] = '\0';
                        renameIndex = -1;
                        focusRename = true;

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel"))
                    {
                        renameIndex = -1;
                        focusRename = true;

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            });

        static BarConfig inspectorBarConfig{BarOrientation::Vertical, "Inspector",
                                            ImVec2(LAYOUT_RIGHT_W, 0.0f), true,
                                            GetDesiredPosition("right")};

        static Bar inspectorBar(inspectorBarConfig);
        inspectorBar.Draw(
            [&]()
            {
                if (selectedGameObjectIndex < 0 ||
                    selectedGameObjectIndex >= static_cast<int>(gameObjects.size()))
                {
                    ImGui::TextDisabled("No GameObject selected");
                    ImGui::Separator();
                    ImGui::TextWrapped(
                        "Select a GameObject in the Hierarchy to edit its properties.");
                    return;
                }

                auto& selected = gameObjects[selectedGameObjectIndex];

                ImGui::Separator();

                ImGui::Checkbox("Active", &selected.isActive);

                ImGui::SameLine();

                ImGui::Text("%s", selected.name.empty()
                                      ? ("GameObject " + std::to_string(selected.id)).c_str()
                                      : selected.name.c_str());

                ImGui::Separator();

                ImGui::TextDisabled("ID: %u", selected.id);
                ImGui::SameLine();
                ImGui::TextDisabled("Pixels: %d", static_cast<int>(selected.pixels.size()));

                ImGui::Separator();

                std::vector<std::string> availableComponents;

                if (!selected.hasComponent<ecs::components::Sprite>())
                {
                    availableComponents.push_back("Sprite");
                }
                else
                {

                    auto s = selected.getComponent<ecs::components::Sprite>();

                    ImGui::Checkbox("##enabledSprite", &s->enabled);
                    ImGui::SameLine();
                    bool open = ImGui::CollapsingHeader("Sprite Component", nullptr,
                                                        ImGuiTreeNodeFlags_DefaultOpen);

                    if (open)
                    {
                        ImGui::BeginDisabled(!s->enabled);

                        if (ImGui::Button("Reset##Sprite"))
                        {
                            // s.texturePath = "";
                            s->width = 640.0f;
                            s->height = 640.0f;
                        }

                        static char textureBuffer[256];
                        std::strncpy(textureBuffer, s->texturePath.c_str(), sizeof(textureBuffer));
                        if (ImGui::InputText("Sprite Texture Path", textureBuffer,
                                             sizeof(textureBuffer)))
                        {
                            s->texturePath = textureBuffer;
                        }
                        ImGui::DragFloat2("Width", &s->width, 640.0f);
                        ImGui::DragFloat2("Height", &s->height, 640.0f);
                        ImGui::DragInt("Layer", &s->layer, 1.0f, -100, 100);

                        ImGui::EndDisabled();

                        if (ImGui::Button("Remove##Sprite"))
                        {
                            selected.removeComponent<ecs::components::Sprite>();
                        }
                    }
                }

                if (!selected.hasComponent<ecs::components::Transform>())
                {
                    availableComponents.push_back("Transform");
                }
                else
                {

                    auto t = selected.getComponent<ecs::components::Transform>();
                    ImGui::Checkbox("##enabledTransform", &t->enabled);
                    ImGui::SameLine();
                    bool open = ImGui::CollapsingHeader("Transform Component", nullptr,
                                                        ImGuiTreeNodeFlags_DefaultOpen);

                    if (open)
                    {
                        ImGui::BeginDisabled(!t->enabled);

                        if (ImGui::Button("Reset##Transform"))
                        {
                            t->x = 0.0f;
                            t->y = 0.0f;
                            t->rotation = 0.0f;
                            t->scaleX = 1.0f;
                            t->scaleY = 1.0f;
                        }

                        ImGui::DragFloat2("Position", &t->x, 0.1f);
                        ImGui::DragFloat("Rotation", &t->rotation, 0.1f);
                        ImGui::DragFloat2("Scale", &t->scaleX, 0.1f);

                        ImGui::EndDisabled();

                        if (ImGui::Button("Remove##Transform"))
                        {
                            selected.removeComponent<ecs::components::Transform>();
                        }
                    }
                }

                if (!selected.hasComponent<ecs::components::Velocity>())
                {
                    availableComponents.push_back("Velocity");
                }
                else
                {
                    auto v = selected.getComponent<ecs::components::Velocity>();

                    ImGui::Checkbox("##enabledVelocity", &v->enabled);
                    ImGui::SameLine();
                    bool open = ImGui::CollapsingHeader("Velocity Component", nullptr,
                                                        ImGuiTreeNodeFlags_DefaultOpen);

                    if (open)
                    {
                        ImGui::BeginDisabled(!v->enabled);

                        if (ImGui::Button("Reset##Velocity"))
                        {
                            v->vx = 0.0f;
                            v->vy = 0.0f;
                        }

                        ImGui::DragFloat2("Velocity", &v->vx, 0.1f);

                        ImGui::EndDisabled();

                        if (ImGui::Button("Remove##Velocity"))
                        {
                            selected.removeComponent<ecs::components::Velocity>();
                        }
                    }
                }

                if (!selected.hasComponent<ecs::components::Script>())
                {
                    availableComponents.push_back("Script");
                }
                else
                {
                    auto s = selected.getComponent<ecs::components::Script>();

                    ImGui::Checkbox("##enabledScript", &s->enabled);
                    ImGui::SameLine();
                    bool open = ImGui::CollapsingHeader("Script Component", nullptr,
                                                        ImGuiTreeNodeFlags_DefaultOpen);

                    if (open)
                    {
                        ImGui::BeginDisabled(!s->enabled);

                        if (ImGui::Button("Reset##Script"))
                        {
                            s->scriptPath = "scripts/movement.lua";
                        }

                        char scriptPathBuffer[512] = {0};
                        std::strncpy(scriptPathBuffer, s->scriptPath.c_str(),
                                     sizeof(scriptPathBuffer) - 1);
                        if (ImGui::InputText("Script Path", scriptPathBuffer,
                                             sizeof(scriptPathBuffer)))
                        {
                            s->scriptPath = scriptPathBuffer;
                        }

                        ImGui::EndDisabled();

                        if (ImGui::Button("Remove##Script"))
                        {
                            selected.removeComponent<ecs::components::Script>();
                        }
                    }
                }

                if (!selected.hasComponent<ecs::components::PhysicsBody>())
                {
                    availableComponents.push_back("PhysicsBody");
                }
                else
                {
                    auto p = selected.getComponent<ecs::components::PhysicsBody>();

                    ImGui::Checkbox("##enabledPhysics", &p->enabled);
                    ImGui::SameLine();
                    bool open = ImGui::CollapsingHeader("Physics Component", nullptr,
                                                        ImGuiTreeNodeFlags_DefaultOpen);

                    if (open)
                    {
                        ImGui::BeginDisabled(!p->enabled);

                        if (ImGui::Button("Reset##Physics"))
                        {
                            p->bodyId = b2_nullBodyId;
                            p->bodyType = b2_dynamicBody;
                            p->fixedRotation = false;
                            p->density = 1.0f;
                            p->friction = 0.4f;
                            p->restitution = 0.1f;
                        }

                        const char* bodyTypes[] = {"Static", "Kinematic", "Dynamic"};
                        int bodyTypeIndex = 2;
                        if (p->bodyType == b2_staticBody)
                            bodyTypeIndex = 0;
                        else if (p->bodyType == b2_kinematicBody)
                            bodyTypeIndex = 1;

                        if (ImGui::Combo("Body Type", &bodyTypeIndex, bodyTypes,
                                         IM_ARRAYSIZE(bodyTypes)))
                        {
                            p->bodyType =
                                bodyTypeIndex == 0
                                    ? b2_staticBody
                                    : (bodyTypeIndex == 1 ? b2_kinematicBody : b2_dynamicBody);
                        }

                        ImGui::Checkbox("Fixed Rotation", &p->fixedRotation);
                        ImGui::DragFloat("Density", &p->density, 0.05f, 0.0f, 100.0f);
                        ImGui::DragFloat("Friction", &p->friction, 0.01f, 0.0f, 1.0f);
                        ImGui::DragFloat("Restitution", &p->restitution, 0.01f, 0.0f, 1.0f);

                        ImGui::TextDisabled("Triangles: %d", static_cast<int>(p->triangles.size()));
                        ImGui::TextDisabled("Body: %s",
                                            B2_IS_NULL(p->bodyId) ? "uncreated" : "created");

                        ImGui::EndDisabled();

                        if (ImGui::Button("Remove##Physics"))
                        {
                            selected.removeComponent<ecs::components::PhysicsBody>();
                        }
                    }
                }

                PopupButton(
                    "Add Component",
                    [&]()
                    {
                        static char search[64] = "";
                        if (ImGui::IsWindowAppearing())
                        {
                            search[0] = '\0';
                        }
                        ImGui::InputText("Search", search, IM_ARRAYSIZE(search));
                        std::string query = search;
                        std::transform(query.begin(), query.end(), query.begin(), ::tolower);

                        for (const auto& comp : availableComponents)
                        {
                            std::string lowerComp = comp;
                            std::transform(lowerComp.begin(), lowerComp.end(), lowerComp.begin(),
                                           ::tolower);

                            if (!query.empty() && lowerComp.find(query) == std::string::npos)
                                continue;

                            if (ImGui::Selectable(comp.c_str()))
                            {
                                if (comp == "Sprite")
                                {
                                    selected.addComponent(ecs::components::Sprite{});
                                    availableComponents.erase(
                                        std::remove(availableComponents.begin(),
                                                    availableComponents.end(), "Sprite"),
                                        availableComponents.end());
                                }
                                else if (comp == "Transform")
                                {
                                    selected.addComponent(ecs::components::Transform{});
                                    availableComponents.erase(
                                        std::remove(availableComponents.begin(),
                                                    availableComponents.end(), "Transform"),
                                        availableComponents.end());
                                }
                                else if (comp == "Velocity")
                                {
                                    selected.addComponent(ecs::components::Velocity{});
                                    availableComponents.erase(
                                        std::remove(availableComponents.begin(),
                                                    availableComponents.end(), "Velocity"),
                                        availableComponents.end());
                                }
                                else if (comp == "PhysicsBody")
                                {
                                    selected.addComponent(ecs::components::PhysicsBody{});
                                    availableComponents.erase(
                                        std::remove(availableComponents.begin(),
                                                    availableComponents.end(), "PhysicsBody"),
                                        availableComponents.end());
                                }
                                else if (comp == "Script")
                                {
                                    selected.addComponent(ecs::components::Script{});
                                    availableComponents.erase(
                                        std::remove(availableComponents.begin(),
                                                    availableComponents.end(), "Script"),
                                        availableComponents.end());
                                }
                                query.clear();
                                search[0] = '\0';
                                break;
                            }
                        }
                    });
            });
    }

    void ImguiInterface::fileToolBar()
    {
        static BarConfig topBarConfig{BarOrientation::Horizontal, "File Toolbar",
                                      ImVec2(0.0f, LAYOUT_TOP_H), true, GetDesiredPosition("top")};

        static Bar topBar(topBarConfig);

        topBar.Draw(
            [&]()
            {
                static int fileIndex = 0;
                std::vector<std::string> fileOptions = {"Save", "Exit"};
                if (DropdownButton("File", fileIndex, fileOptions, fontRegularSmall))
                {
                    switch (fileIndex)
                    {
                        case 0:
                            // Save
                            break;
                        case 1:
                            // Exit
                            break;
                    }
                }

                ImGui::SameLine();

                static int editIndex = 0;
                std::vector<std::string> editOptions = {"Placeholder1", "Placeholder2"}; // temp
                if (DropdownButton("Edit", editIndex, editOptions, fontRegularSmall))
                {
                    switch (editIndex)
                    {
                        case 0:
                            // Placeholder1
                            break;
                        case 1:
                            // Placeholder2
                            break;
                    }
                }

                ImGui::SameLine();

                if (BasicButton("Help", 0.0f, 0.0f, fontRegularSmall))
                {
                    // TODO
                }
            });
    }

    int ImguiInterface::projectOptionsBar(std::vector<projects::Project>& projects,
                                          std::string projectsPath)
    {
        float buttonHeight = 40.0f;
        float buttonWidth = 95.0f;
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float verticalSpacing = 20.0f;

        static bool openNewPopup = false;
        static char projectName[128] = "NewProject";
        static std::filesystem::path selectedPath = projectsPath;
        int resultIndex = -1;

        ImGui::PushFont(fontRegularBig);
        ImGui::Text("Get Started");
        ImGui::PopFont();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(255, 255, 255, 255));
        ImGui::Separator();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();

        if (HoverChangeButton("New", buttonHeight, buttonWidth, fontBoldSmall))
        {
            openNewPopup = true;
        }

        if (openNewPopup)
        {
            ImGui::OpenPopup("Create New Project");
            openNewPopup = false;
        }

        if (ImGui::BeginPopupModal("Create New Project", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText("Project Name", projectName, sizeof(projectName));

            ImGui::Text("Path:\n%s", selectedPath.string().c_str());

            ImGui::SameLine();

            PopupButton(
                "Select Project Path",
                [&]()
                {
                    NFD::UniquePath outPath;

                    nfdresult_t result = NFD::PickFolder(outPath);

                    if (result == NFD_OKAY)
                    {
                        selectedPath = outPath.get();
                    }
                    else if (result == NFD_ERROR)
                    {
                        std::cerr << "Error: " << NFD::GetError() << std::endl;
                    }

                    ImGui::CloseCurrentPopup();
                },
                buttonHeight, buttonWidth, fontRegularSmall);

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Save"))
            {
                std::filesystem::path fullPath = selectedPath / projectName;

                if (!std::filesystem::exists(fullPath))
                {
                    std::filesystem::create_directory(fullPath);
                    std::filesystem::create_directory(fullPath / "Assets");
                    std::filesystem::create_directory(fullPath / "Assets" / "Scenes");

                    fs::path source = std::filesystem::path("assets") / "default.scene";
                    fs::path destination = fullPath / "Assets" / "Scenes" / "default.scene";

                    fs::copy_file(
                        source,
                        destination,
                        fs::copy_options::overwrite_existing
                    );

                    projects::Project newProject;
                    newProject.name = projectName;
                    newProject.path = fullPath;

                    projects.push_back(newProject);

                    newProjectCreated = true;
                }

                ImGui::CloseCurrentPopup();
            }

            if (newProjectCreated)
            {
                newProjectCreated = false;
                resultIndex = projects.size() - 1;
            }

            ImGui::EndPopup();
        }

        ImGui::SameLine(0.0f, verticalSpacing);

        if (HoverChangeButton("Open", buttonHeight, buttonWidth, fontBoldSmall))
        {
            NFD::UniquePath outPath;

            nfdresult_t result = NFD::PickFolder(outPath);

            if (result == NFD_OKAY)
            {
                selectedPath = outPath.get();
            }
            else if (result == NFD_ERROR)
            {
                std::cerr << "Error: " << NFD::GetError() << std::endl;
            }

            projects::Project openedProject;
            openedProject.name = selectedPath.filename().string();
            openedProject.path = selectedPath;
            projects.push_back(openedProject);
            // resultIndex = projects.size() - 1;

        }

        ImGui::SameLine(0.0f, verticalSpacing);

        if (HoverChangeButton("Import", buttonHeight, buttonWidth, fontBoldSmall))
        {
            NFD::UniquePath outPath;

            nfdresult_t result = NFD::PickFolder(outPath);

            if (result == NFD_OKAY)
            {
                selectedPath = outPath.get();

                // TODO: check if it's a valid project

                std::filesystem::path destination =
                    std::filesystem::path(projectsPath) / selectedPath.filename();
                if (std::filesystem::exists(destination))
                {
                    std::cout << "Project already exists in Projects folder.\n";
                    return resultIndex;
                }

                try
                {
                    std::filesystem::rename(selectedPath, destination);

                    projects::Project importedProject;
                    importedProject.name = destination.filename().string();
                    importedProject.path = destination;
                    projects.push_back(importedProject);

                    resultIndex = projects.size() - 1;
                }
                catch (const std::exception& e)
                {
                    std::cout << "Import failed: " << e.what() << std::endl;
                    return resultIndex;
                }
            }
            else if (result == NFD_ERROR)
            {
                std::cout << "Error: " << NFD::GetError() << std::endl;
                return resultIndex;
            }
        }

        ImGui::SameLine(windowWidth);

        if (HoverChangeButton("Tutorial", buttonHeight, buttonWidth, fontBoldSmall))
        {
            // TODO
        }

        return resultIndex;
    }

    int ImguiInterface::projectsDisplay(std::vector<projects::Project>& projects)
    {
        float buttonHeight = 20.0f;
        float buttonWidth = 80.0f;
        float windowWidth = ImGui::GetContentRegionAvail().x;

        ImGui::PushFont(fontRegularBig);
        ImGui::Text("Projects");
        ImGui::PopFont();

        ImGui::SameLine(windowWidth);

        if (BasicButton("ListView", buttonHeight, buttonWidth))
        {
            // TODO
        }

        ImGui::SameLine();

        if (BasicButton("SquareView", buttonHeight, buttonWidth))
        {
            // TODO
        }

        ImGui::Spacing();

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(40, 40, 40, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(70, 70, 70, 255));
        ImGui::BeginChild("ProjectsList", ImVec2(0, 0), true);

        if (projects.empty())
        {
            ImGui::Text("No projects found.");

            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            return -1;
        }

        for (int i = 0; i < static_cast<int>(projects.size()); i++)
        {
            if (clickableProjectOverview(projects[i], fontBoldBig, fontRegularSmall))
            {
                projects[i].lastOpened = std::chrono::system_clock::now();

                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                return i;
            }

            ImGui::Separator();
        }

        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);

        return -1;
    }

    bool ImguiInterface::clickableProjectOverview(projects::Project& project, ImFont* nameFont, ImFont* infoFont)
    {
        ImGui::PushID(project.name.c_str());

        float thumbSize = 200.0f;

        ImVec2 start = ImGui::GetCursorScreenPos();
        float width = ImGui::GetContentRegionAvail().x;
        ImVec2 size(width, thumbSize);

        ImGui::SetCursorScreenPos(start); //
        ImGui::InvisibleButton("clickable project", size);
        ImGui::SetItemAllowOverlap(); //

        bool clicked = ImGui::IsItemClicked();
        bool hovered = ImGui::IsItemHovered();

        ImDrawList* draw = ImGui::GetWindowDrawList();

        ImU32 bgColor = hovered ? IM_COL32(60, 60, 60, 255)
                                : IM_COL32(40, 40, 40, 255);

        draw->AddRectFilled(start,
                            ImVec2(start.x + size.x, start.y + size.y),
                            bgColor, 8.0f);

        ImVec2 imagePos( // movve thumbnail pos
            start.x + 10.0f,
            start.y + (thumbSize / 5.0f)
        );

        if (thumbnail)
        {
            draw->AddImageRounded(
                thumbnail,
                imagePos,
                ImVec2(imagePos.x + thumbSize, imagePos.y + thumbSize),
                ImVec2(0, 0), ImVec2(1, 1),
                IM_COL32_WHITE,
                8.0f
            );
        }
        else
        {
            draw->AddRectFilled(
                imagePos,
                ImVec2(imagePos.x + thumbSize, imagePos.y + thumbSize),
                IM_COL32(80, 80, 80, 255),
                8.0f
            );

            ImVec2 imagePosText(
                imagePos.x + 10.0f,
                imagePos.y + 20.0f
            );
            draw->AddText(
                imagePosText,
                IM_COL32_WHITE,
                "No Img"
            );
        }

        ImVec2 textPos(
            imagePos.x + (thumbSize * 1.2f),
            imagePos.y + (thumbSize / 4.0f)
        );

        if (nameFont) ImGui::PushFont(nameFont);
        draw->AddText(textPos, IM_COL32_WHITE, project.name.c_str());
        if (nameFont) ImGui::PopFont();


        ImVec2 pathTextPos(
            textPos.x,
            textPos.y + (thumbSize / 4.0f) + 6.0f
        );

        if (infoFont) ImGui::PushFont(infoFont);
        draw->AddText(pathTextPos,
                      IM_COL32(180, 180, 180, 255),
                      project.path.string().c_str());
        if (infoFont) ImGui::PopFont();

        std::time_t t = std::chrono::system_clock::to_time_t(project.lastOpened);
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", std::localtime(&t));


        ImVec2 timeTextPos(
            width,
            pathTextPos.y
        );
        
        if (infoFont) ImGui::PushFont(infoFont);
        draw->AddText(timeTextPos,
                      IM_COL32(180, 180, 180, 255),
                      buffer);
        if (infoFont) ImGui::PopFont();

        ImGui::PopID();

        return clicked;
    }

    void ImguiInterface::buildGameSettingsDialog(BuildSettings& settings, bool& confirmed,
                                                 bool& cancelled)
    {
        if (!ImGui::IsPopupOpen("Build Game Settings"))
        {
            settings.syncToBuffers();
            ImGui::OpenPopup("Build Game Settings");
        }

        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        bool open = true;
        if (ImGui::BeginPopupModal("Build Game Settings", &open, ImGuiWindowFlags_None))
        {
            ImGui::Text("Configure your game build:");
            ImGui::Separator();

            ImGui::InputText("Game Title", settings.gameTitleBuf, sizeof(settings.gameTitleBuf));
            ImGui::InputInt("Window Width", &settings.windowWidth);
            ImGui::InputInt("Window Height", &settings.windowHeight);
            ImGui::InputText("Target Name", settings.targetNameBuf, sizeof(settings.targetNameBuf));

            ImGui::Separator();
            ImGui::Text("Output: games/%s/", settings.targetNameBuf);

            ImGui::Separator();
            if (ImGui::Button("Build", ImVec2(120, 0)))
            {
                settings.syncFromBuffers();
                confirmed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                cancelled = true;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void ImguiInterface::showBuildProgressModal(const char* status, float progress, bool isComplete,
                                                bool isSuccess, const char* detail)
    {
        ImGui::SetNextWindowSize(ImVec2(450, 200), ImGuiCond_FirstUseEver);
        bool open = true;
        if (ImGui::BeginPopupModal("Build Progress", &open,
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
        {
            ImGui::Text("%s", status);
            ImGui::Separator();

            if (progress >= 0.0f)
            {
                ImGui::ProgressBar(progress, ImVec2(-1, 0));
            }
            else
            {
                ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(-1, 0), "");
            }

            if (detail && detail[0] != '\0')
            {
                ImGui::Separator();
                ImGui::BeginChild("BuildOutput", ImVec2(0, 80), true,
                                  ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::TextWrapped("%s", detail);
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
                ImGui::EndChild();
            }

            ImGui::Separator();
            if (isComplete)
            {
                if (isSuccess)
                {
                    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "Build successful!");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Build failed.");
                }

                if (ImGui::Button("Close", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }

} // namespace graphics
