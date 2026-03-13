#pragma once

#include <graphics/graphicsEnum.hpp>


namespace graphics {
    class Camera2D {
        public:
            glm::mat4 getViewProjection(int screenWidth, int screenHeight) const;

            void move(glm::vec2 direction);

            void zoomIn(float factor) { zoom *= factor; }
            void zoomOut(float factor) { zoom /= factor; }

            void setPosition(float x, float y);
            void setZoom(float zoom);

            glm::vec2 getPosition() const { return glm::vec2(x, y); }
            float getZoom() const { return zoom; }
            glm::vec2 screenToWorld(const glm::vec2& screenPos, int screenWidth, int screenHeight) const;

        private:
            float x = 0.0f, y = 0.0f;  // center (or camera position) in world coords
            float zoom = 1.0f;         // zoom scale (1.0 = no zoom)
    };
} // namespace graphics