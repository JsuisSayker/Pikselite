#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>      // for glm::ortho, translate, etc.
#include <glm/gtc/type_ptr.hpp> 

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1030

#define PIXEL_SIZE 10.0f

namespace graphics {
    struct Pixel {
        float x, y;
        float r, g, b;
    };

    struct Camera2D {
        float x = 0.0f, y = 0.0f;  // center (or camera position) in world coords
        float zoom = 1.0f;         // zoom scale (1.0 = no zoom)

        glm::mat4 getViewProjection(int screenWidth, int screenHeight) const {
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
    };


    enum InputEventType {
        MOUSE_LEFT_CLICK,
        QUIT,
        NO_EVENT,
    };
}