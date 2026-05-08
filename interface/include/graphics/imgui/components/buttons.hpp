/**
 * @file buttons.hpp
 * @brief Declaration of button components for the ImGui interface.
 */
#pragma once

#include <functional>
#include <imgui.h>
#include <string>
#include <vector>

namespace graphics
{
    bool BasicButton(const std::string& label, float height = 0.0f, float width = 0.0f);
    bool ToggleButton(const std::string& label, bool& value);
    bool DropdownButton(const std::string& label, int& currentIndex,
                        const std::vector<std::string>& options);
    void ColorButton(const std::string& label, ImVec4& color);
    void PopupButton(const std::string& label, const std::function<void()>& contentFunction);
} // namespace graphics