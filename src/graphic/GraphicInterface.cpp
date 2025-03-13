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

    void Graphic::editorSidebar()
    {
        float sidebarHeight = ImGui::GetIO().DisplaySize.y - this->navBarHeight;
        if (!editorSidebarInitialized)
        {
            ImGui::SetNextWindowSize(ImVec2(200, sidebarHeight), ImGuiCond_Always);
            editorSidebarInitialized = true;
        }

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.329f, 0.424f, 0.698f, 1.0f));
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - sidebarHeight), ImGuiCond_Always);

        if (ImGui::Begin("Editor Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration))
        {
            if (ImGui::Button("Open pixel sidebar", ImVec2(180, 40)))
            {
                editorData.showPixelSidebar = !editorData.showPixelSidebar;
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

    void Graphic::pixelSidebar()
    {
        std::cout << "Pixel Sidebar opened" << std::endl; /////////////////////
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
            editorSidebar();

        if (editorData.showColorSelector)
            colorSelector();

        navBar();

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), this->_renderer);
    }

    void Graphic::saveSprite(std::vector<Pixel> pixels, std::string filename)
    {
        SDL_Surface *surface = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0, 0, 0, 0);

        SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 255));

        for (const auto &pixel : pixels)
        {
            SDL_Rect rect = {static_cast<int>(pixel.position.x), static_cast<int>(pixel.position.y), 1, 1};
            SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, pixel.color.r, pixel.color.g, pixel.color.b, pixel.color.a));
        }

        if (IMG_SavePNG(surface, filename.c_str()) != 0)
        {
            std::cerr << "Erreur lors de l'enregistrement de l'image PNG : " << IMG_GetError() << std::endl;
        }

        SDL_FreeSurface(surface);
    }

    void Graphic::createExternalAttributeFile(const std::string &filename, const std::vector<Pixel> &pixels)
    {

        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

        rapidjson::Value pixelArray(rapidjson::kArrayType);

        for (const auto &pixel : pixels)
        {
            rapidjson::Value pixelObj(rapidjson::kObjectType);

            rapidjson::Value posObj(rapidjson::kObjectType);
            posObj.AddMember("x", pixel.position.x, allocator);
            posObj.AddMember("y", pixel.position.y, allocator);
            pixelObj.AddMember("position", posObj, allocator);

            rapidjson::Value colorObj(rapidjson::kObjectType);
            colorObj.AddMember("r", static_cast<int>(pixel.color.r), allocator);
            colorObj.AddMember("g", static_cast<int>(pixel.color.g), allocator);
            colorObj.AddMember("b", static_cast<int>(pixel.color.b), allocator);
            colorObj.AddMember("a", static_cast<int>(pixel.color.a), allocator);
            pixelObj.AddMember("color", colorObj, allocator);

            pixelArray.PushBack(pixelObj, allocator);
        }

        doc.AddMember("pixels", pixelArray, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::ofstream ofs(filename);
        if (!ofs)
        {
            throw std::runtime_error("Could not open file: " + filename);
        }
        ofs << buffer.GetString();
        ofs.close();
    }
}