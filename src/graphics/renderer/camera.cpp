#include <graphics/renderer/camera.hpp>

namespace graphics {
    glm::mat4 Camera2D::getViewProjection(int screenWidth, int screenHeight) const {
        float halfW = (screenWidth * 0.5f) / zoom;
        float halfH = (screenHeight * 0.5f) / zoom;
        
        float left = x - halfW;
        float right = x + halfW;
        float bottom = y - halfH;
        float top = y + halfH;
        
        // Use glm::ortho from <glm/gtc/matrix_transform.hpp>
        glm::mat4 proj = glm::ortho(left, right, bottom, top);
        glm::mat4 view = glm::mat4(1.0f);
        // If you ever add camera translation, you'd do view = translate(...)
        return proj * view;
    }

    void Camera2D::move(glm::vec2 direction) {
        x += direction.x;
        y += direction.y;
    }

    void Camera2D::setPosition(float x, float y) {
        this->x = x;
        this->y = y;
    }

    void Camera2D::setZoom(float zoom) {
        this->zoom = zoom;
    }
} // namespace graphics