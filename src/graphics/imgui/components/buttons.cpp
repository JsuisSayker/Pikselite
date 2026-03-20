/** 
 * @file buttons.cpp
 * @brief Implementation of button components for the ImGui interface.
 * This file contains the definitions for various button types that can be used in the Pixel Engine's UI.
 */

#include <graphics/imgui/components/buttons.hpp>

/** 
 * @brief Draws a basic button that returns true when clicked.
 * @param label The text label for the button.
 * @param height The height of the button.
 * @param width The width of the button.
 * @return true if the button was clicked, false otherwise.
 */
bool graphics::BasicButton(const std::string &label, float height, float width)
{
    if (height > 0.0f && width > 0.0f) {
        return ImGui::Button(label.c_str(), ImVec2(width, height));
    }

    return ImGui::Button(label.c_str());
}

/** 
 * @brief Draws a toggle button (checkbox) that modifies the provided boolean value.
 * @param label The text label for the button.
 * @param value A reference to the boolean value to be modified.
 * @return true if the button was clicked, false otherwise.
 */
bool graphics::ToggleButton(const std::string &label, bool &value)
{
    return ImGui::Checkbox(label.c_str(), &value);
}

/** 
 * @brief Draws a button that, when clicked, shows a dropdown menu with the provided options.
 * @param label The text label for the button.
 * @param currentIndex A reference to the index of the currently selected option.
 * @param options A vector of strings representing the available options.
 * @return true if the selected option was changed, false otherwise.
 */
bool graphics::DropdownButton(const std::string& label, int& currentIndex, const std::vector<std::string>& options)
{
    if (options.empty())
        return false;

    if (currentIndex < 0 || currentIndex >= (int)options.size())
        currentIndex = 0;

    bool changed = false;

    if (ImGui::BeginCombo(label.c_str(), options[currentIndex].c_str()))
    {
        for (size_t i = 0; i < options.size(); i++)
        {
            if (ImGui::Selectable(options[i].c_str(), currentIndex == i))
            {
                currentIndex = i;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }

    return changed;
}

/** 
 * @brief Draws a color button that opens a color picker popup when clicked.
 * @param label The text label for the button.
 * @param color A reference to the ImVec4 value representing the selected color.
 */
void graphics::ColorButton(const std::string &label, ImVec4 &color)
{
    if (ImGui::ColorButton(label.c_str(), color)) {
        ImGui::OpenPopup((label + " Color Picker").c_str());
    }

    if (ImGui::BeginPopup((label + " Color Picker").c_str())) {
        ImGui::ColorPicker4("##picker", (float*)&color);
        ImGui::EndPopup();
    }
}

/** 
 * @brief Draws a button that opens a popup when clicked. The content of the popup is defined by the provided contentFunction.
 * @param label The text label for the button.
 * @param contentFunction A function that defines the content to be displayed in the popup.
 */
void graphics::PopupButton(const std::string& label, const std::function<void()> &contentFunction)
{
    std::string popupId = label + "##popup";

    if (ImGui::Button(label.c_str())) {
        ImGui::OpenPopup(popupId.c_str());
    }

    if (ImGui::BeginPopup(popupId.c_str())) {

        if (contentFunction) {
            contentFunction();
        }

        ImGui::EndPopup();
    }
}