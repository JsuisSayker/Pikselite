#include <graphic/Graphic.hpp>

namespace graphic
{
    void Graphic::colorSelector()
    {
        if (!colorSelectorInitialized)
        {
            ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
            colorSelectorInitialized = true;
        }

        // Begin a window that is resizable and movable by the user
        ImGui::Begin("Color Selector");

        static ImVec4 color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        ImGui::ColorEdit4("Color", (float *)&color);

        editorData.defaultColor = {static_cast<uint8_t>(color.x * 255),
                                   static_cast<uint8_t>(color.y * 255),
                                   static_cast<uint8_t>(color.z * 255),
                                   static_cast<uint8_t>(color.w * 255)};

        ImGui::End();
    }

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
            if (ImGui::Button("Save", ImVec2(180, 40)))
            {
                saveSprite(_pixels, "sprite.png");
                createExternalAttributeFile("sprite.json", _pixels);
            }
        }
        ImGui::End();

        ImGui::PopStyleColor();
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
        }
        ImGui::End();

        ImGui::PopStyleColor();
    }

    void Graphic::addFileExplorer()
    {
        // Open the file dialog if needed. Note: It’s best to call OpenDialog only when you really want to open it.
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".json, .png,.jpg,.jpeg,.bmp,.tga,.gif,.psd,.hdr,.pic");

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    projectData.spritePath = filePathName;
                }
                ImGuiFileDialog::Instance()->Close();
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }

    void Graphic::pixelEditorSidebar()
    {
        float sidebarWidth = 200.0f;
        float sidebarHeight = ImGui::GetIO().DisplaySize.y - this->navBarHeight;
        if (!pixelEditorSidebarInitialized)
        {
            ImGui::SetNextWindowSize(ImVec2(200, sidebarHeight), ImGuiCond_Always);
            pixelEditorSidebarInitialized = true;
        }

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - sidebarWidth, ImGui::GetIO().DisplaySize.y - sidebarHeight), ImGuiCond_Always);

        if (ImGui::Begin("Pixel Editor Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration))
        {
            ImGui::Text("Color Selector");
            ImGui::BeginChild("Color Selector Child", ImVec2(ImGui::GetContentRegionAvail().x, 200), true, ImGuiWindowFlags_NoScrollbar);
            static ImVec4 color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            ImGui::ColorPicker4("##color", (float *)&color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
            ImGui::EndChild();

            editorData.defaultColor = {
                static_cast<uint8_t>(color.x * 255),
                static_cast<uint8_t>(color.y * 255),
                static_cast<uint8_t>(color.z * 255),
                static_cast<uint8_t>(color.w * 255)};

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
        }
        ImGui::End();

        ImGui::PopStyleColor();
    }

    void Graphic::lightOptions()
    {
        ImGui::OpenPopup("Light Options", ImGuiWindowFlags_AlwaysAutoResize);
        float popupWidth = 300.0f;
        float popupHeight = 300.0f;
        ImGui::SetNextWindowSize(ImVec2(popupWidth, popupHeight), ImGuiCond_Appearing);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2));

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
                lightData.radius = newRadius;
                lightData.intensity = newIntensity;
                ImGui::CloseCurrentPopup();
                newRadius = lightData.radius;
                newIntensity = lightData.intensity;
                editorData.showLightOptions = false;
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
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2));

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
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2));

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
                liquidData.viscosity = newViscosity;
                ImGui::CloseCurrentPopup();
                newViscosity = liquidData.viscosity;
                editorData.showLiquidOptions = false;
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

    void Graphic::drawInterface()
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

        if (editorData.showColorSelector)
            colorSelector();

        if (editorData.showPixelEditorSidebar)
            pixelEditorSidebar();

        if (editorData.showLightOptions)
            lightOptions();

        if (editorData.showSolidOptions)
            solidOptions();
        
        if (editorData.showLiquidOptions)
            liquidOptions();

        navBar();

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), this->_renderer);
    }
}