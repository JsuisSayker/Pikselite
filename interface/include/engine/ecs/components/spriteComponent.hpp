#pragma once

#include <string>
#include <GL/glew.h>

namespace ecs::components
{
    struct Sprite {
        std::string texturePath = "assets/dragon.png";
        GLuint      textureID   = 0;       // filled at runtime by the render system
        float       width       = 640.0f;
        float       height      = 640.0f;
        bool        loaded      = false;   // true once the texture has been loaded
    };
} // namespace ecs::components
