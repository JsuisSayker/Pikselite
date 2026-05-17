#include <graphics/renderer/camera.hpp>

namespace graphics
{
    glm::mat4 Camera2D::getViewProjection(int screenWidth, int screenHeight) const
    {
        float halfW = (screenWidth * 0.5f) / zoom;
        float halfH = (screenHeight * 0.5f) / zoom;

        float left = x - halfW;
        float right = x + halfW;
        float bottom = y - halfH;
        float top = y + halfH;

        glm::mat4 proj = glm::ortho(left, right, bottom, top);
        glm::mat4 view = glm::mat4(1.0f);
        return proj * view;
    }

    void Camera2D::move(glm::vec2 direction)
    {
        x += direction.x;
        y += direction.y;
    }

    void Camera2D::setPosition(float x, float y)
    {
        this->x = x;
        this->y = y;
    }

    void Camera2D::setZoom(float zoom)
    {
        this->zoom = zoom;
    }

    void Camera2D::zoomAt(float factor, const glm::vec2& screenPos, int screenWidth,
                          int screenHeight)
    {
        glm::vec2 worldBefore = screenToWorld(screenPos, screenWidth, screenHeight);
        zoom *= factor;
        if (zoom < 0.05f)
            zoom = 0.05f;
        if (zoom > 50.0f)
            zoom = 50.0f;
        glm::vec2 worldAfter = screenToWorld(screenPos, screenWidth, screenHeight);
        x += worldBefore.x - worldAfter.x;
        y += worldBefore.y - worldAfter.y;
    }

    glm::vec2 Camera2D::screenToWorld(const glm::vec2& screenPos, int screenWidth,
                                      int screenHeight) const
    {
        float halfW = (screenWidth * 0.5f) / zoom;
        float halfH = (screenHeight * 0.5f) / zoom;

        // X: left -> right
        float worldX = (x - halfW) + (screenPos.x / zoom);

        // Y: invert screen Y (top-left origin) to world Y-up
        float worldY = (y + halfH) - (screenPos.y / zoom);

        return glm::vec2(worldX, worldY);
    }
} // namespace graphics