/**
 * @file graphicsEnum.hpp
 * @brief Declares graphics-related enums and structs for the Pixel Engine project, including definitions for pixels, sprites, and input events.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and interactions.
 */
#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>      // for glm::ortho, translate, etc.
#include <glm/gtc/type_ptr.hpp> 

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1030

#define PIXEL_SIZE 10.0f

/**
 * @brief The graphics namespace contains classes and functions related to rendering and user interface management, including definitions for pixels, sprites, and input events used throughout the rendering and editing systems of the Pixel Engine project.
 */
namespace graphics {
    /**
     * @brief Struct representing a pixel to be rendered, containing its position in world coordinates and its color as an RGB vector. This struct is used for managing the pixels that will be rendered in the scene, allowing for easy storage and manipulation of pixel data during rendering and editing processes.
     * position: A glm::vec2 representing the x and y coordinates of the pixel in world space.
     * color: A glm::vec3 representing the RGB color of the pixel, with each
     */
    struct Pixel {
        glm::vec2 position;
        glm::vec3 color;
    };

    /**
     * @brief Struct representing a vertex for line rendering, containing its position in world coordinates and its color as an RGB vector. This struct is used for managing the vertices of lines that will be rendered in the scene, allowing for easy storage and manipulation of line vertex data during rendering processes.
     * position: A glm::vec2 representing the x and y coordinates of the vertex in world space.
     * color: A glm::vec3 representing the RGB color of the vertex, with each component in the range [0.0, 1.0].
     */
    struct LineVertex {
        glm::vec2 position;
        glm::vec3 color;
    };

    /**
     * @brief Struct representing a 2D sprite, containing its position and size in world coordinates, as well as the OpenGL texture ID for rendering. This struct is used for managing sprites in the scene, allowing for easy storage and manipulation of sprite data during rendering processes.
     * position: A glm::vec2 representing the x and y coordinates of the sprite's anchor point in world space.
     * size: A glm::vec2 representing the width and height of the sprite in world units.
     * textureID: An OpenGL texture ID representing the texture to be used for rendering the sprite. This ID should correspond to a valid texture that has been loaded into OpenGL, and will be used when drawing the sprite to the screen.
     */
    struct Sprite2D {
        glm::vec2 position;
        glm::vec2 size;
        GLuint textureID = 0;
    };

    /**
     * @brief Enum representing different types of user input events that can be processed by the graphics interface and editors. This enum is used for managing user input in a consistent way across the application, allowing for easy handling of keyboard and mouse events, as well as window events such as closing the window or quitting the application.
     */
    enum InputEventType {
        KEY_W,
        KEY_A,
        KEY_S,
        KEY_D,
        KEY_I,
        KEY_K,
        KEY_L,
        KEY_O,
        KEY_TAB,
        KEY_F5,
        MOUSE_LEFT_CLICK,
        MOUSE_LEFT_DRAG,
        MOUSE_RIGHT_CLICK,
        MOUSE_RIGHT_DRAG,
        WINDOW_CLOSE,
        QUIT,
        NO_EVENT,
    };

    /**
     * @brief Struct representing a user input event, containing the type of event and the associated window ID. This struct is used for managing user input events in the graphics interface and editors, allowing for easy storage and processing of input events as they are polled from the SDL event queue.
     * type: An InputEventType enum value representing the type of user input event that occurred.
     * windowID: A uint32_t representing the ID of the window associated with the event, which can be used to determine which window received the input event in cases where multiple windows are present.
     */
    struct InputEvent {
        InputEventType type = NO_EVENT;
        uint32_t windowID = 0;
    };
}