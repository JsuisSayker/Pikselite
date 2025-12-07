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

} // namespace graphics