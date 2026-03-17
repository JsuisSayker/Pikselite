#include <graphics/imgui/components/bars.hpp>

// ── Layout constants (shared with interface.cpp via bars.hpp) ──────────────
static constexpr float BOTTOM_H = 180.0f;

void graphics::Bar::Draw(const std::function<void()> &contentFunction) {
    if (!config.visible) return;

    ImGuiIO &io = ImGui::GetIO();
    const float winW = io.DisplaySize.x;
    const float winH = io.DisplaySize.y;

    // ── Compute non-overlapping position & size ──────────────────────────
    ImVec2 finalPos  = config.position;
    ImVec2 finalSize = config.size;

    const bool isTopHorizontal = (config.orientation == BarOrientation::Horizontal && config.position.y >= 0.0f);

    if (config.orientation == BarOrientation::Horizontal) {
        const bool isBottom = (config.position.y < 0.0f);
        if (isBottom) {
            // Bottom bar: full width, fixed height, snapped to bottom
            finalPos  = ImVec2(0.0f, winH - BOTTOM_H);
            finalSize = ImVec2(winW,  BOTTOM_H);
        } else {
            // Top toolbar: full width
            finalPos  = ImVec2(0.0f, 0.0f);
            finalSize = ImVec2(winW, LAYOUT_TOP_H);
        }
    } else {
        // Vertical sidebar: fixed width, height stops above the bottom bar
        float sideH = winH - BOTTOM_H - LAYOUT_TOP_H;
        if (finalPos.x < 0.0f)
            finalPos.x = winW - finalSize.x; // right side
        finalPos.y  = LAYOUT_TOP_H;
        finalSize.y = sideH;
    }

    // ── Window flags: immovable, no resize, no collapse ──────────────────
    ImGuiWindowFlags panelFlags =
        ImGuiWindowFlags_NoMove               |
        ImGuiWindowFlags_NoResize             |
        ImGuiWindowFlags_NoCollapse           |
        ImGuiWindowFlags_NoBringToFrontOnFocus|
        ImGuiWindowFlags_NoTitleBar           |
        ImGuiWindowFlags_NoSavedSettings;

    // Top toolbars should never scroll.
    if (isTopHorizontal) {
        panelFlags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    }

    // ── Unity-inspired dark theme ─────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_WindowBg,       ImVec4(0.16f, 0.16f, 0.16f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg,        ImVec4(0.13f, 0.13f, 0.13f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Border,         ImVec4(0.06f, 0.06f, 0.06f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Header,         ImVec4(0.26f, 0.59f, 0.98f, 0.31f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,  ImVec4(0.26f, 0.59f, 0.98f, 0.50f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,   ImVec4(0.26f, 0.59f, 0.98f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_Button,         ImVec4(0.26f, 0.26f, 0.26f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.46f, 0.46f, 0.46f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.10f, 0.10f, 0.10f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.20f, 0.20f, 0.20f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.24f, 0.24f, 0.24f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Separator,      ImVec4(0.08f, 0.08f, 0.08f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg,    ImVec4(0.10f, 0.10f, 0.10f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab,  ImVec4(0.30f, 0.30f, 0.30f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.40f, 0.40f, 0.40f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ResizeGrip,     ImVec4(0.00f, 0.00f, 0.00f, 0.00f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,  0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,   ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,     ImVec2(6.0f, 5.0f));

    ImGui::SetNextWindowPos(finalPos,  ImGuiCond_Always);
    ImGui::SetNextWindowSize(finalSize, ImGuiCond_Always);

    // Draw a tab-like header manually since NoTitleBar is set
    ImGui::Begin(config.label.c_str(), nullptr, panelFlags);

    // Top bars are compact toolbars: no extra header row.
    if (!isTopHorizontal) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.00f));
        ImGui::TextUnformatted(config.label.c_str());
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();
    }

    if (contentFunction) contentFunction();

    ImGui::End();

    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(17);
}