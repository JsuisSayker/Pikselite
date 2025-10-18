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
        glm::vec2 position;
        glm::vec3 color;
    };

    struct LineVertex {
        glm::vec2 position;
        glm::vec3 color;
    };

    enum InputEventType {
        KEY_W,
        KEY_A,
        KEY_S,
        KEY_D,
        KEY_I,
        KEY_O,
        MOUSE_LEFT_CLICK,
        QUIT,
        NO_EVENT,
    };
}