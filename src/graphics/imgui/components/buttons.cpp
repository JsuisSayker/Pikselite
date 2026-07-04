/**
 * @file buttons.cpp
 * @brief Implementation of button components for the ImGui interface.
 * This file contains the definitions for various button types that can be used in the Pixel
 * Engine's UI.
 */

#include <graphics/imgui/components/buttons.hpp>

/**
 * @brief Draws a basic button that returns true when clicked.
 * @param label The text label for the button.
 * @param height The height of the button.
 * @param width The width of the button.
 * @return true if the button was clicked, false otherwise.
 */
bool graphics::BasicButton(const std::string& label, float height, float width, ImFont* font)
{
    if (font)
        ImGui::PushFont(font);

    bool clicked;
    if (height > 0.0f && width > 0.0f)
        clicked = ImGui::Button(label.c_str(), ImVec2(width, height));
    else
        clicked = ImGui::Button(label.c_str());

    if (font)
        ImGui::PopFont();

    return clicked;
}

/**
 * @brief Draws a toggle button (checkbox) that modifies the provided boolean value.
 * @param label The text label for the button.
 * @param value A reference to the boolean value to be modified.
 * @return true if the button was clicked, false otherwise.
 */
bool graphics::ToggleButton(const std::string& label, bool& value, ImFont* font)
{
    if (font)
        ImGui::PushFont(font);
    bool result = ImGui::Checkbox(label.c_str(), &value);
    if (font)
        ImGui::PopFont();
    return result;
}

/**
 * @brief Draws a button that, when clicked, shows a dropdown menu with the provided options.
 * @param label The text label for the button.
 * @param currentIndex A reference to the index of the currently selected option.
 * @param options A vector of strings representing the available options.
 * @return true if the selected option was changed, false otherwise.
 */
bool graphics::DropdownButton(const std::string& label, int& selectedIndex,
                              const std::vector<std::string>& options, ImFont* font)
{
    if (options.empty())
        return false;

    selectedIndex = -1;

    std::string popupId = label + "##dropdown";

    if (font)
        ImGui::PushFont(font);

    if (ImGui::Button(label.c_str()))
    {
        ImGui::OpenPopup(popupId.c_str());
    }

    if (font)
        ImGui::PopFont();

    if (ImGui::BeginPopup(popupId.c_str()))
    {
        if (font)
            ImGui::PushFont(font);

        for (int i = 0; i < (int)options.size(); i++)
        {
            if (ImGui::Selectable(options[i].c_str()))
            {
                selectedIndex = i;
                ImGui::CloseCurrentPopup();
            }
        }

        if (font)
            ImGui::PopFont();

        ImGui::EndPopup();
    }

    return selectedIndex != -1;
}

/**
 * @brief Draws a color button that opens a color picker popup when clicked.
 * @param label The text label for the button.
 * @param color A reference to the ImVec4 value representing the selected color.
 */
void graphics::ColorButton(const std::string& label, ImVec4& color, ImFont* font)
{
    std::string popupId = label + " Color Picker";

    if (font)
        ImGui::PushFont(font);

    if (ImGui::ColorButton(label.c_str(), color))
    {
        ImGui::OpenPopup(popupId.c_str());
    }

    if (font)
        ImGui::PopFont();

    if (ImGui::BeginPopup(popupId.c_str()))
    {

        if (font)
            ImGui::PushFont(font);

        ImGui::ColorPicker4("##picker", (float*)&color);

        if (font)
            ImGui::PopFont();

        ImGui::EndPopup();
    }
}

/**
 * @brief Draws a button that opens a popup when clicked. The content of the popup is defined by the
 * provided contentFunction.
 * @param label The text label for the button.
 * @param contentFunction A function that defines the content to be displayed in the popup.
 */
void graphics::PopupButton(const std::string& label, const std::function<void()>& contentFunction,
                           float height, float width, ImFont* font)
{
    std::string popupId = label + "##popup";

    if (font)
        ImGui::PushFont(font);

    if (ImGui::Button(label.c_str()))
    {
        ImGui::OpenPopup(popupId.c_str());
    }

    if (font)
        ImGui::PopFont();

    if (ImGui::BeginPopup(popupId.c_str()))
    {

        if (font)
            ImGui::PushFont(font);

        if (contentFunction)
        {
            contentFunction();
        }

        if (font)
            ImGui::PopFont();

        ImGui::EndPopup();
    }
}

bool graphics::HoverChangeButton(const std::string& label, float height, float width, ImFont* font)
{
    if (font)
        ImGui::PushFont(font);

    ImVec4 bgNormal = ImVec4(0, 0, 0, 0);
    ImVec4 bgHover = ImVec4(1, 1, 1, 1);
    ImVec4 bgActive = ImVec4(0.85f, 0.85f, 0.85f, 1);

    ImVec4 borderCol = ImVec4(0.6f, 0.6f, 0.6f, 1);

    ImVec4 textNormal = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    ImVec4 textHover = ImVec4(0.1f, 0.1f, 0.1f, 1);

    ImGui::PushStyleColor(ImGuiCol_Button, bgNormal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgActive);
    ImGui::PushStyleColor(ImGuiCol_Border, borderCol);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    bool clicked = ImGui::Button(label.c_str(), ImVec2(width, height));

    ImU32 textColor = ImGui::GetColorU32(ImGui::IsItemHovered() ? textHover : textNormal);

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImVec2 textSize = ImGui::CalcTextSize(label.c_str());

    ImVec2 center = ImVec2(min.x + (max.x - min.x - textSize.x) * 0.5f,
                           min.y + (max.y - min.y - textSize.y) * 0.5f);

    ImGui::GetWindowDrawList()->AddText(center, textColor, label.c_str());

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);

    if (font)
        ImGui::PopFont();

    return clicked;
}