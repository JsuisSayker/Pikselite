/**
 * @file buttons.hpp
 * @brief Declaration of button components for the ImGui interface.
 */
#pragma once

#include <functional>
#include <imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <string>
#include <vector>

namespace graphics
{
    bool BasicButton(const std::string& label, float height = 0.0f, float width = 0.0f,
                     ImFont* font = nullptr);
    bool ToggleButton(const std::string& label, bool& value, ImFont* font = nullptr);
    bool DropdownButton(const std::string& label, int& currentIndex,
                        const std::vector<std::string>& options, ImFont* font = nullptr);
    void ColorButton(const std::string& label, ImVec4& color, ImFont* font = nullptr);
    void PopupButton(const std::string& label, const std::function<void()>& contentFunction,
                     float height = 0.0f, float width = 0.0f, ImFont* font = nullptr);
    bool HoverChangeButton(const std::string& label, float height, float width,
                           ImFont* font = nullptr);
} // namespace graphics