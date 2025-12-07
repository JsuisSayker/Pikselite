#include <graphics/imgui/components/bars.hpp>

void graphics::Bar::Draw(const std::function<void()> &contentFunction) {
    if (!config.visible) {
        return;
    }

    if (config.position.x < 0) {
        ImGuiIO &io = ImGui::GetIO();
        float winW = io.DisplaySize.x;
        ImGui::SetNextWindowPos(ImVec2(
            winW - config.size.x,
            config.position.y
        ));
    } else {
        ImGui::SetNextWindowPos(config.position);
    }

    if (config.orientation == BarOrientation::Horizontal) {
        ImGui::SetNextWindowSize(ImVec2(
            ImGui::GetIO().DisplaySize.x,
            config.size.y
        ));
    } else {
        ImGui::SetNextWindowSize(ImVec2(
            config.size.x,
            ImGui::GetIO().DisplaySize.y - config.position.y
        ));
    }

    ImGui::Begin(config.label.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if (contentFunction) {
        contentFunction();
    }
    
    ImGui::End();
}