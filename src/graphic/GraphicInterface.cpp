#include <graphic/Graphic.hpp>

namespace graphic
{
    void Graphic::navBar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 10.0f));

        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))
                {
                }
                if (ImGui::MenuItem("Open"))
                {
                }
                if (ImGui::MenuItem("Exit"))
                {
                }
                if (ImGui::MenuItem("Save"))
                {
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo"))
                {
                }
                if (ImGui::MenuItem("Redo"))
                {
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About"))
                {
                }
                ImGui::EndMenu();
            }

            float spacing = ImGui::GetContentRegionAvail().x - 5.0f;
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + spacing);

            if (ImGui::SmallButton("X"))
            {
                _windowOpen = false;
            }

            this->navBarHeight = ImGui::GetWindowHeight();

            ImGui::EndMainMenuBar();
        }

        ImGui::PopStyleColor();

        ImGui::PopStyleVar();
    }

    std::unordered_map<std::string, std::string> getSpriteFilesName(const std::string &path)
    {
        std::unordered_map<std::string, std::string> files;
        try
        {
            for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(path))
            {
                if (std::filesystem::is_regular_file(entry.status()))
                {
                    files[entry.path().filename().string()] = entry.path().string();
                }
            }
        }
        catch (const std::filesystem::filesystem_error &ex)
        {
            std::cerr << "Error accessing directory: " << ex.what() << std::endl;
        }
        return files;
    }

    void Graphic::spriteEditorSidebar()
    {
        float sidebarHeight = ImGui::GetIO().DisplaySize.y - this->navBarHeight;
        if (!spriteEditorSidebarInitialized)
        {
            ImGui::SetNextWindowSize(ImVec2(200, sidebarHeight), ImGuiCond_Always);
            spriteEditorSidebarInitialized = true;
        }

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - sidebarHeight), ImGuiCond_Always);

        if (ImGui::Begin("Sprite Editor Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration))
        {
            ImGui::PushFont(this->_iconFont);
            if (ImGui::Button(ICON_FA_PAINT_BRUSH, ImVec2(180, 40)))
            {
                editorData.showPixelEditorSidebar = !editorData.showPixelEditorSidebar;
                this->pixelEditorSidebarInitialized = false;
                this->selectedPixel = nullptr;
            }
            ImGui::PopFont();

            if (ImGui::Button("Reset Camera", ImVec2(180, 40)))
            {
                editorData.resetCamera = true;
            }
            if (ImGui::Checkbox("Show grid", &editorData.showGrid))
            {
            }
            if (ImGui::Checkbox("Auto link", &editorData.autoLink))
            {
            }
            if (ImGui::Button("Clear", ImVec2(180, 40)))
            {
                _pixels.clear();
            }
            if (ImGui::Button("Export Sprite", ImVec2(180, 40)))
            {
                projectData.showDirectoryChooser = !projectData.showDirectoryChooser;
            }
        }
        ImGui::End();

        ImGui::PopStyleColor();
    }

    void Graphic::spriteInputSidebar()
    {
        float sidebarWidth = 200.0f;
        float sidebarHeight = ImGui::GetIO().DisplaySize.y - this->navBarHeight;
        if (!spriteEditorSidebarInitialized)
        {
            ImGui::SetNextWindowSize(ImVec2(200, sidebarHeight), ImGuiCond_Always);
            spriteEditorSidebarInitialized = true;
        }

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - sidebarWidth, ImGui::GetIO().DisplaySize.y - sidebarHeight), ImGuiCond_Always);

        if (ImGui::Begin("Sprite Editor Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration))
        {
            ImGui::PushFont(this->_iconFont);
            if (ImGui::Button(ICON_FA_PAINT_BRUSH, ImVec2(180, 40)))
            {
                editorData.showPixelEditorSidebar = !editorData.showPixelEditorSidebar;
            }
            ImGui::PopFont();

            if (ImGui::Button("Reset Camera", ImVec2(180, 40)))
            {
                editorData.resetCamera = true;
            }
            if (ImGui::Checkbox("Show grid", &editorData.showGrid))
            {
            }
            if (ImGui::Checkbox("Auto link", &editorData.autoLink))
            {
            }
            if (ImGui::Button("Clear", ImVec2(180, 40)))
            {
                _pixels.clear();
            }
            if (ImGui::Button("Export Sprite", ImVec2(180, 40)))
            {
                projectData.showDirectoryChooser = !projectData.showDirectoryChooser;
            }
        }
        ImGui::End();

        ImGui::PopStyleColor();
    }

    void getSpriteFromFileName(graphic::ProjectEditorData *projectData)
    {
        static int currentItem = 0;
        int index = 0;

        // Change the string to the path of the sprites folder of the user
        std::unordered_map<std::string, std::string> items = getSpriteFilesName("sprites");

        for (std::unordered_map<std::string, std::string>::iterator it = items.begin(); it != items.end(); ++it)
        {
            bool is_selected = (currentItem == index);
            if (ImGui::Selectable(it->first.c_str(), is_selected))
            {
                currentItem = index;
                projectData->showImportSprite = !projectData->showImportSprite;
                if (it->second.find(".json") == std::string::npos)
                {
                    projectData->oldSpritePath = it->second;
                    std::string newFileName = it->second.substr(0, it->second.find_last_of('.'));
                    newFileName += ".json";
                    it->second = newFileName;
                }
                else
                {
                    std::string newFileName = it->second.substr(0, it->second.find_last_of('.'));
                    newFileName += ".png";
                    projectData->oldSpritePath = newFileName;
                }
                projectData->spritePath = it->second;
                projectData->selectedFileName = it->first;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();

            index++;
        }
        ImGui::EndCombo();
    }

    void Graphic::projectEditorSidebar()
    {
        float sidebarHeight = ImGui::GetIO().DisplaySize.y - this->navBarHeight;
        if (!projectEditorSidebarInitialized)
        {
            ImGui::SetNextWindowSize(ImVec2(200, sidebarHeight), ImGuiCond_Always);
            projectEditorSidebarInitialized = true;
        }

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - sidebarHeight), ImGuiCond_Always);

        if (ImGui::Begin("Project Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration))
        {
            if (ImGui::Button("Import Sprite", ImVec2(180, 40)))
            {
                projectData.showFileExplorer = !projectData.showFileExplorer;
            }
            if (ImGui::Checkbox("Show grid", &projectData.showGrid))
            {
            }
            if (ImGui::BeginCombo("Sprites", projectData.selectedFileName.c_str()))
            {
                getSpriteFromFileName(&projectData);
            }
            if (ImGui::Button("run", ImVec2(180, 40)))
            {
                projectData.runGame = true;
            }
        }
        ImGui::End();

        ImGui::PopStyleColor();
    }

    void Graphic::addExportFileExplorer()
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png, .json");

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                projectData.folderPath = filePathName;
                ImGuiFileDialog::Instance()->Close();
                projectData.showDirectoryChooser = false;
            }
            else
            {
                ImGuiFileDialog::Instance()->Close();
                projectData.showDirectoryChooser = false;
            }
        }
    }

    void Graphic::addFileExplorer()
    {
        // Open the file dialog if needed. Note: It’s best to call OpenDialog only when you really want to open it.
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".json, .png");

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                projectData.spritePath = filePathName;
                ImGuiFileDialog::Instance()->Close();
                projectData.showFileExplorer = false;
            }
            else
            {
                ImGuiFileDialog::Instance()->Close();
                projectData.showFileExplorer = false;
            }
        }
    }

    void Graphic::pixelEditorSidebar()
    {
        float sidebarWidth = 200.0f;
        float sidebarHeight = ImGui::GetIO().DisplaySize.y - this->navBarHeight;
        if (!pixelEditorSidebarInitialized)
        {
            ImGui::SetNextWindowSize(ImVec2(200, sidebarHeight), ImGuiCond_Always);
        }

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - sidebarWidth, ImGui::GetIO().DisplaySize.y - sidebarHeight), ImGuiCond_Always);

        if (ImGui::Begin("Pixel Editor Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration))
        {
            ImGui::Text("Color Selector");
            ImGui::BeginChild("Color Selector Child", ImVec2(ImGui::GetContentRegionAvail().x, 200), true, ImGuiWindowFlags_NoScrollbar);
            static ImVec4 color;
            if (!pixelEditorSidebarInitialized)
            {
                color = ImVec4(
                    editorData.defaultColor.r / 255.0f,
                    editorData.defaultColor.g / 255.0f,
                    editorData.defaultColor.b / 255.0f,
                    editorData.defaultColor.a / 255.0f);

                pixelEditorSidebarInitialized = true;
            }

            ImGui::ColorPicker4("##color", (float *)&color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
            ImGui::EndChild();

            if (selectedPixel == nullptr)
            {
                editorData.defaultColor = {
                    static_cast<uint8_t>(color.x * 255),
                    static_cast<uint8_t>(color.y * 255),
                    static_cast<uint8_t>(color.z * 255),
                    static_cast<uint8_t>(color.w * 255)};
            }
            else
            {
                selectedPixel->color = {
                    static_cast<uint8_t>(color.x * 255),
                    static_cast<uint8_t>(color.y * 255),
                    static_cast<uint8_t>(color.z * 255),
                    static_cast<uint8_t>(color.w * 255)};
            }

            ImGui::Separator();

            ImGui::Text("Status");
            if (ImGui::Button("Light", ImVec2(180, 40)))
            {
                editorData.showLightOptions = true;
            }
            if (ImGui::Button("Solid", ImVec2(180, 40)))
            {
                editorData.showSolidOptions = true;
            }
            if (ImGui::Button("Liquid", ImVec2(180, 40)))
            {
                editorData.showLiquidOptions = true;
            }

            ImGui::Separator();

            if (selectedPixel)
            {
                ImGui::Text("Selected Pixel:\n(%f, %f)", selectedPixel->position.x, selectedPixel->position.y);
            }
        }
        ImGui::End();

        ImGui::PopStyleColor();
    }

    ImTextureID LoadTextureFromFile(const char *filename, SDL_Renderer *renderer)
    {
        // Check if the filename is valid (non-null and not empty)
        if (!filename || std::string(filename).empty())
        {
            std::cerr << "Empty filename provided!" << std::endl;
            return ImTextureID(0);
        }

        // Convert to absolute path using std::filesystem
        std::filesystem::path absPath = std::filesystem::absolute(filename);
        std::string basePath = absPath.string();

        // Check file accessibility
        std::ifstream file(basePath);
        if (!file)
        {
            std::cerr << "File not accessible!" << std::endl;
            return ImTextureID(0);
        }

        // Load the image using SDL_image
        SDL_Surface *loadedSurface = IMG_Load(basePath.c_str());
        if (!loadedSurface)
        {
            std::cerr << "Unable to load image " << filename << "! SDL_image Error: "
                      << IMG_GetError() << std::endl;
            return ImTextureID(0);
        }

        // Create an SDL_Texture from the loaded surface
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (!texture)
        {
            std::cerr << "Unable to create texture from " << filename
                      << "! SDL Error: " << SDL_GetError() << std::endl;
            SDL_FreeSurface(loadedSurface);
            return ImTextureID(0);
        }

        // Free the loaded surface
        SDL_FreeSurface(loadedSurface);

        // Return the texture as ImTextureID (using SDL_Texture* directly)
        return reinterpret_cast<ImTextureID>(texture);
    }

    void Graphic::spriteSelector(Camera camera)
    {
        // If the texture hasn't been loaded yet, load it.
        if (projectData.dragImagetextureId == 0)
            projectData.dragImagetextureId = LoadTextureFromFile(projectData.oldSpritePath.c_str(), this->_renderer);

        // Retrieve the texture's dimensions using SDL_QueryTexture.
        int texW = 0, texH = 0;
        SDL_Texture *texture = reinterpret_cast<SDL_Texture *>(projectData.dragImagetextureId);
        if (SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH) != 0)
        {
            std::cerr << "Failed to query texture: " << SDL_GetError() << std::endl;
            texW = texH = 200;
        }
        ImVec2 imageSize(static_cast<float>(texW), static_cast<float>(texH));

        // Push style colors for the tooltip.
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(1, 1, 1, 0));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern))
        {
            ImGui::SetDragDropPayload("DND_DEMO_CELL", &projectData.dragImagetextureId, sizeof(ImTextureID));

            float zoomLevel = camera.zoom;
            ImVec2 zoomedSize = ImVec2(imageSize.x * zoomLevel, imageSize.y * zoomLevel);

            ImVec2 mousePos = ImGui::GetIO().MousePos;
            // Offset the tooltip window so that the image center is at the mouse position.
            ImVec2 tooltipPos = ImVec2(mousePos.x - zoomedSize.x * 0.5f,
                                       (mousePos.y - zoomedSize.y * 0.5f));
            ImGui::SetNextWindowPos(tooltipPos, ImGuiCond_Always);

            // Remove default window padding for the tooltip to ensure pixel-perfect centering.
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginTooltip();
            ImGui::Image(projectData.dragImagetextureId, zoomedSize, ImVec2(0, 0), ImVec2(1, 1));
            ImGui::EndTooltip();
            ImGui::PopStyleVar();

            ImGui::EndDragDropSource();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }

    void Graphic::lightOptions()
    {
        ImGui::OpenPopup("Light Options", ImGuiWindowFlags_AlwaysAutoResize);
        float popupWidth = 300.0f;
        float popupHeight = 300.0f;
        ImGui::SetNextWindowSize(ImVec2(popupWidth, popupHeight), ImGuiCond_Appearing);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - popupWidth - 280, 300));

        if (ImGui::BeginPopup("Light Options"))
        {
            static int newRadius = 0;
            static int newIntensity = 0;

            ImGui::Text("Light Options");

            ImGui::SliderInt("##radius_slider", &newRadius, 0, 100);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(popupWidth / 3.0f);
            ImGui::InputInt("##radius_input", &newRadius);

            ImGui::SliderInt("##intensity_slider", &newIntensity, 0, 100);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(popupWidth / 3.0f);
            ImGui::InputInt("##intensity_input", &newIntensity);

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
                newRadius = lightData.radius;
                newIntensity = lightData.intensity;
                editorData.showLightOptions = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Save"))
            {
                if (selectedPixel == nullptr)
                {
                    lightData.radius = newRadius;
                    lightData.intensity = newIntensity;
                }
                else
                {
                    selectedPixel->attributes.push_back(light{newRadius, newIntensity});
                }

                ImGui::CloseCurrentPopup();
                newRadius = lightData.radius;
                newIntensity = lightData.intensity;
                editorData.showLightOptions = false;
            }
            ImGui::SameLine();

            if (selectedPixel == nullptr)
            {
                if (ImGui::Checkbox("Enabled", &editorData.lightEnabled))
                {
                    for (Pixel &pixel : _pixels)
                    {
                        pixel.lightEnabled = editorData.lightEnabled;
                    }
                }
            }
            else
            {
                if (ImGui::Checkbox("Enabled", &selectedPixel->lightEnabled))
                {
                }
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleColor();
    }

    void Graphic::solidOptions()
    {
        ImGui::OpenPopup("Solid Options", ImGuiWindowFlags_AlwaysAutoResize);
        float popupWidth = 300.0f;
        float popupHeight = 300.0f;
        ImGui::SetNextWindowSize(ImVec2(popupWidth, popupHeight), ImGuiCond_Appearing);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - popupWidth - 280, 300));

        if (ImGui::BeginPopup("Solid Options"))
        {
            ImGui::Text("Solid Options");

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
                editorData.showSolidOptions = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Save"))
            {
                ImGui::CloseCurrentPopup();
                editorData.showSolidOptions = false;
            }
            ImGui::SameLine();

            if (selectedPixel == nullptr)
            {
                if (ImGui::Checkbox("Enabled", &editorData.solidEnabled))
                {
                    for (Pixel &pixel : _pixels)
                    {
                        pixel.solidEnabled = editorData.solidEnabled;
                    }
                }
            }
            else
            {
                if (ImGui::Checkbox("Enabled", &selectedPixel->solidEnabled))
                {
                }
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleColor();
    }

    void Graphic::liquidOptions()
    {
        ImGui::OpenPopup("Liquid Options", ImGuiWindowFlags_AlwaysAutoResize);
        float popupWidth = 300.0f;
        float popupHeight = 300.0f;
        ImGui::SetNextWindowSize(ImVec2(popupWidth, popupHeight), ImGuiCond_Appearing);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - popupWidth - 280, 300));

        if (ImGui::BeginPopup("Liquid Options"))
        {
            static int newViscosity = 0;

            ImGui::Text("liquid Options");

            ImGui::SliderInt("##viscosity_slider", &newViscosity, 0, 100);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(popupWidth / 3.0f);
            ImGui::InputInt("##viscosity_input", &newViscosity);

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
                newViscosity = liquidData.viscosity;
                editorData.showLiquidOptions = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Save"))
            {
                if (selectedPixel == nullptr)
                {
                    liquidData.viscosity = newViscosity;
                }
                else
                {
                    selectedPixel->attributes.push_back(liquid{newViscosity});
                }
                ImGui::CloseCurrentPopup();
                newViscosity = liquidData.viscosity;
                editorData.showLiquidOptions = false;
            }
            ImGui::SameLine();

            if (selectedPixel == nullptr)
            {
                if (ImGui::Checkbox("Enabled", &editorData.liquidEnabled))
                {
                    for (Pixel &pixel : _pixels)
                    {
                        pixel.liquidEnabled = editorData.liquidEnabled;
                    }
                }
            }
            else
            {
                if (ImGui::Checkbox("Enabled", &selectedPixel->liquidEnabled))
                {
                }
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleColor();
    }

    void Graphic::homeInterface()
    {
        float menuBarHeight = 33.0f;

        ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
        ImGui::SetNextWindowSize(ImVec2((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT - menuBarHeight));

        ImGui::Begin("Home",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove);

        ImGui::Text("Get started");

        if (ImGui::Button("New"))
        {
            showHome = false;
        }
        ImGui::SameLine();

        if (ImGui::Button("Import"))
        {
        }
        ImGui::SameLine();

        if (ImGui::Button("Tutorial"))
        {
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Recent Projects");

        ImGui::BeginChild("RecentProjects", ImVec2(0, 200), true);
        ImGui::Text("All recent and past projects will appear here");
        ImGui::EndChild();

        ImGui::End();
    }

    void Graphic::drawInterface(Camera camera)
    {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (showHome)
            homeInterface();

        if (isSpriteEditor)
            spriteEditorSidebar();

        if (isProjectEditor)
            projectEditorSidebar();

        if (projectData.showFileExplorer)
            addFileExplorer();

        if (projectData.showDirectoryChooser)
            addExportFileExplorer();

        if (editorData.showPixelEditorSidebar)
            pixelEditorSidebar();

        if (projectData.showImportSprite)
            spriteSelector(camera);

        if (editorData.showLightOptions)
            lightOptions();

        if (editorData.showSolidOptions)
            solidOptions();

        if (editorData.showLiquidOptions)
            liquidOptions();

        if (projectData.showSpriteSelector)
            spriteInputSidebar();

        navBar();

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), this->_renderer);
    }
}