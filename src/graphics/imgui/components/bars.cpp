#include <graphics/imgui/components/bars.hpp>

void graphics::Bar::Draw(const std::function<void()> &contentFunction) {
    if (!config.visible) {
        return;
    }

    ImGuiIO &io = ImGui::GetIO();

    float winW = io.DisplaySize.x;
    float winH = io.DisplaySize.y;

    ImVec2 pos = config.position;

    // Right
    if (pos.x < 0) {
        pos.x = winW - config.size.x;
    }

    // Bottom
    if (pos.y < 0) {
        pos.y = winH - config.size.y;
    }

    ImGui::SetNextWindowPos(pos);

    if (config.orientation == BarOrientation::Horizontal) {
        ImGui::SetNextWindowSize(ImVec2(
            winW,
            config.size.y
        ));
    } 
    else {
        ImGui::SetNextWindowSize(ImVec2(
            config.size.x,
            winH
        ));
    }

    ImGui::Begin(config.label.c_str(), nullptr);

    if (contentFunction) {
        contentFunction();
    }

    ImGui::End();
}