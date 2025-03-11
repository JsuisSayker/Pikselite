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
            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleColor();

        ImGui::PopStyleVar();
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

        if (showColorSelector)
            colorSelector();

        navBar();

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), this->_renderer);
    }
}