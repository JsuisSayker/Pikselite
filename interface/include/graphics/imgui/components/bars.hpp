#pragma once

#include <imgui.h>
#include <string>
#include <functional>

namespace graphics {
    enum class BarOrientation {
        Horizontal,
        Vertical
    };

    struct BarConfig {
        BarOrientation orientation = BarOrientation::Horizontal;
        std::string label;
        ImVec2 size = ImVec2(0, 0);
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

     static ImVec2 GetDesiredPosition(std::string position) {
        if (position == "top") {
            return ImVec2(0, 0);
        } else if (position == "bottom") {
            return ImVec2(0, -1);
        } else if (position == "left") {
            return ImVec2(0, 0);
        } else if (position == "right") {
            return ImVec2(-1, 0);
        } else {
            return ImVec2(0, 0);
        }
    }

    static int GetDesiredSize(std::string size, BarOrientation orientation) {
        if (size == "full" && orientation == BarOrientation::Horizontal) {
            return ImGui::GetIO().DisplaySize.x;
        } else if (size == "full" && orientation == BarOrientation::Vertical) {
            return ImGui::GetIO().DisplaySize.y;
        } else {
            return 0;
        }
    }

} // namespace graphics