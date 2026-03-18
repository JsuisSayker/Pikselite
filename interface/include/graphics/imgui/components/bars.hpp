#pragma once

#include <imgui.h>
#include <string>
#include <functional>

namespace graphics {

    // ── Editor Mode ──────────────────────────────────────────────────────────────
    enum class EditorMode {
        SpriteEditor,
        ProjectEditor
    };

    // ── Layout constants ──────────────────────────────────────────────────────────
    //  Used by Bar::Draw and optionally by OpenGL viewport code.
    static constexpr float LAYOUT_TOP_H     = 40.0f;  // Tab bar height
    static constexpr float LAYOUT_BOTTOM_H  = 180.0f;
    static constexpr float LAYOUT_LEFT_W    = 220.0f;
    static constexpr float LAYOUT_RIGHT_W   = 260.0f;

    enum class BarOrientation {
        Horizontal,
        Vertical
    };

    struct BarConfig {
        BarOrientation orientation = BarOrientation::Horizontal;
        std::string label;
        ImVec2 size = ImVec2(0, 0); // width (vertical) or height (horizontal) in .x/.y
        bool visible = true;
        ImVec2 position = ImVec2(0, 0);
    };

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
    static ImVec2 GetDesiredPosition(const std::string& position) {
        if (position == "right")  return ImVec2(-1.0f, 0.0f);  // x<0 → right-anchored
        if (position == "bottom") return ImVec2( 0.0f,-1.0f);  // y<0 → bottom-anchored
        return ImVec2(0.0f, 0.0f);                              // left / top
    }

    // Fixed dimension of a slot (width for vertical bars, height for horizontal).
    static int GetDesiredSize(const std::string& slot, BarOrientation /*orientation*/) {
        if (slot == "full") return 0; // Bar::Draw ignores stored size and computes
        return 0;
    }

} // namespace graphics