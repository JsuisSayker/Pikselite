#include <graphics/imgui/components/buttons.hpp>

// Draws a basic button that returns true when clicked.
bool graphics::BasicButton(const std::string &label, float height, float width)
{
    if (height > 0.0f && width > 0.0f) {
        return ImGui::Button(label.c_str(), ImVec2(width, height));
    }

    return ImGui::Button(label.c_str());
}

// Draws a toggle button (checkbox) that modifies the provided boolean value. Returns true when clicked.
bool graphics::ToggleButton(const std::string &label, bool &value)
{
    return ImGui::Checkbox(label.c_str(), &value);
}

// Draws a button that, when clicked, shows a dropdown menu with the provided options. When an option is clicked, the currentIndex is updated to the selected option.
void graphics::DropdownButton(const std::string &label, int &currentIndex, const std::vector<std::string> &options)
{
    if (options.empty())
        return;

    if (currentIndex < 0 || currentIndex >= (int)options.size())
        currentIndex = 0;
    
    if (ImGui::BeginCombo(label.c_str(), options[currentIndex].c_str())) {
        for (size_t i = 0; i < options.size(); i++) {
           if ((ImGui::Selectable(options[i].c_str(), currentIndex == i))) {
               currentIndex = i;
           }
        }
        ImGui::EndCombo();
    }
}

// Draws a color button that opens a color picker popup when clicked. The selected color is stored in the provided ImVec4 reference.
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

// Draws a button that opens a popup when clicked. The content of the popup is defined by the provided contentFunction.
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