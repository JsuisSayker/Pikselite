/**
 * @file camera.hpp
 * @brief Declares the Camera2D class for managing the 2D camera view and transformations.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and
 * interactions
 */
#pragma once

#include <graphics/graphicsEnum.hpp>

/**
 * @brief The graphics namespace contains classes and functions related to rendering and user
 * interface management. The Camera2D class provides methods for calculating the view-projection
 * matrix, moving the camera
 */
namespace graphics
{
    class Camera2D
    {
      public:
        /**
         * @brief Calculates and returns the view-projection matrix for the camera, based on its
         * position and zoom level.
         * @param screenWidth The width of the screen in pixels.
         * @param screenHeight The height of the screen in pixels.
         * @return A glm::mat4 representing the combined view and projection transformation for the
         * camera
         */
        glm::mat4 getViewProjection(int screenWidth, int screenHeight) const;

        /**
         * @brief Moves the camera by the specified direction vector, which is in world coordinates.
         * @param direction A glm::vec2 representing the direction and magnitude to move the camera
         * Note: The direction vector should be scaled appropriately based on the current zoom level
         * to achieve consistent movement speed
         */
        void move(glm::vec2 direction);

        // Methods for zooming the camera in and out by a specified factor
        void zoomIn(float factor)
        {
            zoom *= factor;
        }
        void zoomOut(float factor)
        {
            zoom /= factor;
        }

        /**
         * @brief Zooms the camera while keeping the world point under the given screen position
         * stable. Useful for mouse-wheel zoom that focuses on the cursor.
         */
        void zoomAt(float factor, const glm::vec2& screenPos, int screenWidth, int screenHeight);

        // Accessor and mutator methods for camera position and zoom level
        void setPosition(float x, float y);
        void setZoom(float zoom);

        // Getters for camera position and zoom level
        glm::vec2 getPosition() const
        {
            return glm::vec2(x, y);
        }
        float getZoom() const
        {
            return zoom;
        }

        /**
         * @brief Converts screen coordinates (e.g., from mouse input) to world coordinates based on
         * the camera's position and zoom level.
         * @param screenPos A glm::vec2 containing the x and y coordinates in screen space (origin
         * at top-left corner).
         * @param screenWidth The width of the screen in pixels.
         * @param screenHeight The height of the screen in pixels.
         * @return A glm::vec2 containing the corresponding x and y coordinates in world space,
         * taking into account the camera's position and zoom level.
         */
        glm::vec2 screenToWorld(const glm::vec2& screenPos, int screenWidth,
                                int screenHeight) const;

      private:
        // Camera position in world coordinates (center of the view)
        float x = 0.0f, y = 0.0f; // center (or camera position) in world coords
        // Zoom level of the camera (1.0 = no zoom, >1.0 = zoom in, <1.0 = zoom out)
        float zoom = 1.0f; // zoom scale (1.0 = no zoom)
    };
} // namespace graphics