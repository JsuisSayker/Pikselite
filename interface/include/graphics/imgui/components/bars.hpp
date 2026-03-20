/**
    * @file bars.hpp
    * @brief Declaration of the Bar class for rendering customizable UI bars (toolbars and sidebars) using ImGui.
    * This file defines the Bar class and its configuration structure, which are used to create and
 */
#pragma once

#include <imgui.h>
#include <string>
#include <functional>

namespace graphics {

    // ── Editor Mode ──────────────────────────────────────────────────────────────
    /** 
     * @brief Enum class for specifying the mode of the editor.
     */
    enum class EditorMode {
        SpriteEditor,
        ProjectEditor
    };

    // ── Layout constants ──────────────────────────────────────────────────────────
    //  Used by Bar::Draw and optionally by OpenGL viewport code.
    static constexpr float LAYOUT_TOP_H     = 40.0f;  // Top toolbar height
    static constexpr float LAYOUT_BOTTOM_H  = 180.0f;
    static constexpr float LAYOUT_LEFT_W    = 220.0f;
    static constexpr float LAYOUT_RIGHT_W   = 260.0f;

    /** 
     * @brief Enum class for specifying the orientation of a UI bar.
     */
    enum class BarOrientation {
        Horizontal,
        Vertical
    };

    /** 
     * @brief Configuration structure for a UI bar, containing properties such as orientation, label, size, visibility, and position. This structure is used to initialize and configure instances of the Bar class.
     */
    struct BarConfig {
        BarOrientation orientation = BarOrientation::Horizontal;
        std::string label;
        ImVec2 size = ImVec2(0, 0); // width (vertical) or height (horizontal) in .x/.y
        bool visible = true;
        ImVec2 position = ImVec2(0, 0);
    };

    /** 
     * @brief Class for creating and managing customizable UI bars (toolbars and sidebars) using ImGui.
     */
    class Bar {
    public:
        Bar(const BarConfig &config) : config(config) {}
        void Draw(const std::function<void()> &contentFunction);
        bool IsVisible() const { return config.visible; }
    private:
        BarConfig config;
    };

    // ── Slot helpers ─────────────────────────────────────────────────────────
    //  Pass "left", "right", "top", or "bottom".
    //  Bar::Draw overrides final size/position at draw-time using LAYOUT_* consts.
    /** 
     * @brief Gets the desired position for a UI bar based on its slot.
     * @param position The slot for the bar ("left", "right", "top", or "bottom").
     * @return The desired position as an ImVec2.
     */
    static ImVec2 GetDesiredPosition(const std::string& position) {
        if (position == "right")  return ImVec2(-1.0f, 0.0f);  // x<0 → right-anchored
        if (position == "bottom") return ImVec2( 0.0f,-1.0f);  // y<0 → bottom-anchored
        return ImVec2(0.0f, 0.0f);                              // left / top
    }

    // Fixed dimension of a slot (width for vertical bars, height for horizontal).
    /** 
     * @brief Gets the desired size for a UI bar based on its slot and orientation.
     * @param slot The slot for the bar ("full", "left", "right", "top", or "bottom").
     * @param orientation The orientation of the bar (horizontal or vertical).
     * @return The desired size as a float (width for vertical bars, height for horizontal bars).
     */
    static int GetDesiredSize(const std::string& slot, BarOrientation /*orientation*/) {
        if (slot == "full") return 0; // Bar::Draw ignores stored size and computes
        return 0;
    }

} // namespace graphics