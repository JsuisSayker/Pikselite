#pragma once

#include <GL/glew.h>
#include <string>

namespace ecs::components
{
    /**
     * @brief The Sprite component represents a 2D image that can be rendered on the screen.
     * It contains properties such as the texture path, texture ID, dimensions, and an enabled flag.
     */
    struct Sprite
    {
        bool enabled = true;

        std::string texturePath = "assets/dragon.png";
        GLuint      textureID   = 0; // filled at runtime by the render system
        float       width       = 640.0f;
        float       height      = 640.0f;
        bool        loaded      = false; // true once the texture has been loaded
    };
} // namespace ecs::components
