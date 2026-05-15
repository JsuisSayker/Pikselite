#include <box2d/collision.h>
#include <box2d/math_functions.h>
#include <graphics/renderer/renderer.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Pixel shaders
const char* vertexShaderSrc = R"(
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 uVP;         // camera View × Projection
uniform float uPointSize;
uniform bool uUseCamera;  // if true, apply camera; else, screen-space pass-through
uniform vec2 uScreenSize; // width, height of window

out vec3 vColor;

void main() {
    if (uUseCamera) {
        gl_Position = uVP * vec4(aPos, 0.0, 1.0);
    } else {
        // screen‐space overlay: aPos is in pixel coordinates (0..width, 0..height)
        vec2 ndc = aPos / uScreenSize * 2.0 - 1.0;
        ndc.y = -ndc.y;  // flip Y because window origin is usually top-left
        gl_Position = vec4(ndc, 0.0, 1.0);
    }
    gl_PointSize = uPointSize;
    vColor = aColor;
}
)";

const char* fragmentShaderSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

// Sprite shaders
const char* spriteVertexShaderSrc = R"(
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uVP;
uniform mat4 uModel;

out vec2 vTexCoord;

void main() {
    gl_Position = uVP * uModel * vec4(aPos, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

const char* spriteFragmentShaderSrc = R"(
#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    FragColor = texture(uTexture, vTexCoord);
}
)";

namespace graphics
{
    Renderer::Renderer(SDL_Window* window, SDL_GLContext glContext)
        : _window(window), _glContext(glContext)
    {
        int width, height;
        SDL_GetWindowSize(_window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        glEnable(GL_PROGRAM_POINT_SIZE);
        glDisable(GL_DEPTH_TEST);

        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);

        initShader();
        initSpriteShader();
    }

    Renderer::~Renderer()
    {
        glDeleteProgram(_shader);
        glDeleteProgram(_spriteShader);
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_spriteVbo);
        glDeleteVertexArrays(1, &_vao);
        glDeleteVertexArrays(1, &_spriteVao);
    }

    void checkShaderCompile(GLuint shader, const char* name)
    {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "Shader compilation failed (" << name << "):\n" << infoLog << std::endl;
        }
    }

    void checkProgramLink(GLuint program)
    {
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Program linking failed:\n" << infoLog << std::endl;
        }
    }

    void Renderer::initShader()
    {
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertexShaderSrc, nullptr);
        glCompileShader(vs);
        checkShaderCompile(vs, "Vertex");

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragmentShaderSrc, nullptr);
        glCompileShader(fs);
        checkShaderCompile(fs, "Fragment");

        _shader = glCreateProgram();
        glAttachShader(_shader, vs);
        glAttachShader(_shader, fs);
        glLinkProgram(_shader);
        checkProgramLink(_shader);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    void Renderer::initSpriteShader()
    {
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &spriteVertexShaderSrc, nullptr);
        glCompileShader(vs);
        checkShaderCompile(vs, "SpriteVertex");

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &spriteFragmentShaderSrc, nullptr);
        glCompileShader(fs);
        checkShaderCompile(fs, "SpriteFragment");

        _spriteShader = glCreateProgram();
        glAttachShader(_spriteShader, vs);
        glAttachShader(_spriteShader, fs);
        glLinkProgram(_spriteShader);
        checkProgramLink(_spriteShader);

        glDeleteShader(vs);
        glDeleteShader(fs);

        // Setup sprite VAO/VBO (quad: 2 triangles, 4 vertices)
        // position (vec2) + texcoord (vec2)
        float quadVertices[] = {
            // pos      // tex
            -0.5f, 0.5f,  0.0f, 1.0f, // top-left
            -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
            0.5f,  -0.5f, 1.0f, 0.0f, // bottom-right
            0.5f,  0.5f,  1.0f, 1.0f  // top-right
        };

        unsigned int indices[] = {0, 1, 2, 0, 2, 3};

        GLuint ebo;
        glGenVertexArrays(1, &_spriteVao);
        glGenBuffers(1, &_spriteVbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(_spriteVao);

        glBindBuffer(GL_ARRAY_BUFFER, _spriteVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // texcoord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void Renderer::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer::present(SDL_Window* window)
    {
        SDL_GL_SwapWindow(window);
    }

    void Renderer::drawPixelsWCamera(const std::vector<Pixel>& pixels, const Camera2D& camera,
                                     float pixelSize)
    {
        if (pixels.empty())
            return;

        int width, height;
        SDL_GetWindowSize(_window, &width, &height);

        glUseProgram(_shader);

        // set uniforms for camera mode
        GLint useCamLoc = glGetUniformLocation(_shader, "uUseCamera");
        if (useCamLoc != -1)
            glUniform1i(useCamLoc, GL_TRUE);

        // Upload VP matrix
        glm::mat4 vp = camera.getViewProjection(width, height);
        GLint vpLoc = glGetUniformLocation(_shader, "uVP");
        if (vpLoc != -1)
            glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(vp));

        // Screen size (for fallback pass-through mode, but still good to set it)
        GLint screenLoc = glGetUniformLocation(_shader, "uScreenSize");
        if (screenLoc != -1)
            glUniform2f(screenLoc, (float)width, (float)height);

        // Point size
        float effectivePointSize = pixelSize * camera.getZoom();
        GLint sizeLoc = glGetUniformLocation(_shader, "uPointSize");
        if (sizeLoc != -1)
            glUniform1f(sizeLoc, effectivePointSize);

        // Upload vertex data
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, pixels.size() * sizeof(Pixel), pixels.data(),
                     GL_DYNAMIC_DRAW);

        // Attribute layout
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Pixel),
                              reinterpret_cast<void*>(offsetof(Pixel, position)));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Pixel),
                              reinterpret_cast<void*>(offsetof(Pixel, color)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_POINTS, 0, (GLsizei)pixels.size());

        glBindVertexArray(0);
    }

    void Renderer::drawParticlesWCamera(const std::vector<Element::Particle>& particles,
                                        const Camera2D& camera, float pixelSize)
    {
        if (particles.empty())
            return;

        std::vector<Pixel> pixels;
        pixels.reserve(particles.size());

        for (const auto& particle : particles)
        {
            if (particle.type == Element::EMPTY)
                continue;

            const auto& def = g_elements[particle.type];
            const auto& palette = def.colorPalette[particle.colorIndex % PALETTE_SIZE];

            Pixel p;
            p.position = {particle.position.x * PIXEL_SIZE, particle.position.y * PIXEL_SIZE};
            p.color = {palette.r / 255.0f, palette.g / 255.0f, palette.b / 255.0f};
            pixels.push_back(p);
        }

        drawPixelsWCamera(pixels, camera, pixelSize);
    }

    void Renderer::drawPixelsOverlay(const std::vector<Pixel>& pixels, float pixelSize)
    {
        if (pixels.empty())
            return;

        int width, height;
        SDL_GetWindowSize(_window, &width, &height);

        glUseProgram(_shader);

        // set uniforms for overlay mode
        GLint useCamLoc = glGetUniformLocation(_shader, "uUseCamera");
        if (useCamLoc != -1)
            glUniform1i(useCamLoc, GL_FALSE);

        // We may skip setting uVP (not used when uUseCamera = false) but it's safe to provide
        // Also set screen size so the pass-through code works
        GLint screenLoc = glGetUniformLocation(_shader, "uScreenSize");
        if (screenLoc != -1)
            glUniform2f(screenLoc, (float)width, (float)height);

        // Set point size
        GLint sizeLoc = glGetUniformLocation(_shader, "uPointSize");
        if (sizeLoc != -1)
            glUniform1f(sizeLoc, pixelSize);

        // Upload vertex data
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, pixels.size() * sizeof(Pixel), pixels.data(),
                     GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Pixel),
                              reinterpret_cast<void*>(offsetof(Pixel, position)));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Pixel),
                              reinterpret_cast<void*>(offsetof(Pixel, color)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_POINTS, 0, (GLsizei)pixels.size());

        glBindVertexArray(0);
    }

    void Renderer::drawGrid(const Camera2D& camera, float cellSize, glm::vec3 color)
    {
        const float zoom = camera.getZoom();
        if (zoom <= 0.0f)
            return;

        // When cells become too small on screen, the fine grid becomes visual noise
        // that obscures the work. Switch to a power-of-two coarser grid so the
        // spatial reference is preserved without overwhelming the view.
        constexpr float FINE_THRESHOLD_PX = 6.0f;
        constexpr float COARSE_TARGET_PX = 24.0f;

        float drawCellSize = cellSize;
        if (cellSize * zoom < FINE_THRESHOLD_PX)
        {
            int multiplier = 1;
            while ((cellSize * static_cast<float>(multiplier) * zoom) < COARSE_TARGET_PX &&
                   multiplier < 4096)
            {
                multiplier *= 2;
            }
            drawCellSize = cellSize * static_cast<float>(multiplier);
        }

        int width, height;
        SDL_GetWindowSize(_window, &width, &height);

        // Compute world bounds visible through the camera
        const float halfW = (width * 0.5f) / zoom;
        const float halfH = (height * 0.5f) / zoom;
        const glm::vec2 camPos = camera.getPosition();

        const float left = camPos.x - halfW;
        const float right = camPos.x + halfW;
        const float bottom = camPos.y - halfH;
        const float top = camPos.y + halfH;

        // Iterate by integer cell index to avoid float-accumulation drift during zoom,
        // and pad by one cell on each side so lines don't pop in/out at the visible edges.
        const int startCellX = static_cast<int>(std::floor(left / drawCellSize)) - 1;
        const int endCellX = static_cast<int>(std::ceil(right / drawCellSize)) + 1;
        const int startCellY = static_cast<int>(std::floor(bottom / drawCellSize)) - 1;
        const int endCellY = static_cast<int>(std::ceil(top / drawCellSize)) + 1;

        const float lineTop = top + drawCellSize;
        const float lineBottom = bottom - drawCellSize;
        const float lineLeft = left - drawCellSize;
        const float lineRight = right + drawCellSize;

        // Use fine cellSize/2 offset (not drawCellSize/2) so coarse grid lines still
        // sit on fine pixel boundaries when the user zooms back in.
        const float lineOffset = cellSize * 0.5f;

        std::vector<LineVertex> vertices;
        vertices.reserve(static_cast<size_t>((endCellX - startCellX + endCellY - startCellY) * 2));

        for (int i = startCellX; i <= endCellX; ++i)
        {
            float x = static_cast<float>(i) * drawCellSize + lineOffset;
            vertices.push_back({{x, lineBottom}, color});
            vertices.push_back({{x, lineTop}, color});
        }

        for (int i = startCellY; i <= endCellY; ++i)
        {
            float y = static_cast<float>(i) * drawCellSize + lineOffset;
            vertices.push_back({{lineLeft, y}, color});
            vertices.push_back({{lineRight, y}, color});
        }

        if (vertices.empty())
            return;

        glUseProgram(_shader);

        // Set camera mode
        GLint useCamLoc = glGetUniformLocation(_shader, "uUseCamera");
        if (useCamLoc != -1)
            glUniform1i(useCamLoc, GL_TRUE);

        // Upload camera VP matrix
        glm::mat4 vp = camera.getViewProjection(width, height);
        GLint vpLoc = glGetUniformLocation(_shader, "uVP");
        if (vpLoc != -1)
            glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(vp));

        // Upload vertices
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(LineVertex), vertices.data(),
                     GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                              reinterpret_cast<void*>(offsetof(LineVertex, position)));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                              reinterpret_cast<void*>(offsetof(LineVertex, color)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_LINES, 0, (GLsizei)vertices.size());
        glBindVertexArray(0);
    }

    void Renderer::drawSegments(const std::vector<LineVertex>& segments, const Camera2D& camera)
    {
        if (segments.empty())
            return;

        int width, height;
        SDL_GetWindowSize(_window, &width, &height);

        glUseProgram(_shader);

        GLint useCamLoc = glGetUniformLocation(_shader, "uUseCamera");
        if (useCamLoc != -1)
            glUniform1i(useCamLoc, GL_TRUE);

        glm::mat4 vp = camera.getViewProjection(width, height);
        GLint vpLoc = glGetUniformLocation(_shader, "uVP");
        if (vpLoc != -1)
            glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(vp));

        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, segments.size() * sizeof(LineVertex), segments.data(),
                     GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                              reinterpret_cast<void*>(offsetof(LineVertex, position)));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                              reinterpret_cast<void*>(offsetof(LineVertex, color)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_LINES, 0, (GLsizei)segments.size());
        glBindVertexArray(0);
    }

    void Renderer::drawBox2DDebug(b2WorldId worldId, const std::vector<b2BodyId>& bodies,
                                  const Camera2D& camera, float pixelsPerMeter, glm::vec3 color)
    {
        if (!b2World_IsValid(worldId))
            return;
        if (bodies.empty())
            return;

        std::vector<LineVertex> segments;
        segments.reserve(bodies.size() * 16);

        for (b2BodyId bodyId : bodies)
        {
            if (!b2Body_IsValid(bodyId))
                continue;

            int shapeCount = b2Body_GetShapeCount(bodyId);
            if (shapeCount <= 0)
                continue;

            std::vector<b2ShapeId> shapes(shapeCount);
            int actualCount = b2Body_GetShapes(bodyId, shapes.data(), shapeCount);
            b2Transform xf = b2Body_GetTransform(bodyId);

            for (int i = 0; i < actualCount; ++i)
            {
                b2ShapeId shapeId = shapes[i];
                if (!b2Shape_IsValid(shapeId))
                    continue;

                b2ShapeType type = b2Shape_GetType(shapeId);
                if (type == b2_polygonShape)
                {
                    b2Polygon poly = b2Shape_GetPolygon(shapeId);
                    b2Polygon worldPoly = b2TransformPolygon(xf, &poly);

                    for (int v = 0; v < worldPoly.count; ++v)
                    {
                        b2Vec2 p1 = worldPoly.vertices[v];
                        b2Vec2 p2 = worldPoly.vertices[(v + 1) % worldPoly.count];

                        segments.push_back(
                            {glm::vec2(p1.x * pixelsPerMeter, p1.y * pixelsPerMeter), color});
                        segments.push_back(
                            {glm::vec2(p2.x * pixelsPerMeter, p2.y * pixelsPerMeter), color});
                    }
                }
                else if (type == b2_segmentShape)
                {
                    b2Segment seg = b2Shape_GetSegment(shapeId);
                    b2Vec2 p1 = b2TransformPoint(xf, seg.point1);
                    b2Vec2 p2 = b2TransformPoint(xf, seg.point2);

                    segments.push_back(
                        {glm::vec2(p1.x * pixelsPerMeter, p1.y * pixelsPerMeter), color});
                    segments.push_back(
                        {glm::vec2(p2.x * pixelsPerMeter, p2.y * pixelsPerMeter), color});
                }
            }
        }

        drawSegments(segments, camera);
    }

    GLuint Renderer::loadTexture(const std::string& filePath)
    {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Texture wrapping/filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Pixel-perfect
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Load image with stb_image
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true); // Flip to match OpenGL Y-up
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            std::cerr << "Failed to load texture: " << filePath << std::endl;
            glDeleteTextures(1, &textureID);
            return 0;
        }

        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);

        std::cout << "Loaded texture: " << filePath << " (" << width << "x" << height << ", "
                  << channels << " channels)\n";
        return textureID;
    }

    void Renderer::drawSprite(const Sprite2D& sprite, const Camera2D& camera)
    {
        if (sprite.textureID == 0)
            return;

        int width, height;
        SDL_GetWindowSize(_window, &width, &height);

        glUseProgram(_spriteShader);

        // Enable alpha blending for transparent PNGs
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // VP matrix
        glm::mat4 vp = camera.getViewProjection(width, height);
        GLint vpLoc = glGetUniformLocation(_spriteShader, "uVP");
        if (vpLoc != -1)
            glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(vp));

        // Model matrix: translate to position, scale to size
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(sprite.position, 0.0f));
        model = glm::scale(model, glm::vec3(sprite.size, 1.0f));

        GLint modelLoc = glGetUniformLocation(_spriteShader, "uModel");
        if (modelLoc != -1)
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sprite.textureID);
        GLint texLoc = glGetUniformLocation(_spriteShader, "uTexture");
        if (texLoc != -1)
            glUniform1i(texLoc, 0);

        // Draw quad
        glBindVertexArray(_spriteVao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDisable(GL_BLEND);
    }

    void Renderer::unloadTexture(GLuint textureID)
    {
        if (textureID != 0)
            glDeleteTextures(1, &textureID);
    }
} // namespace graphics
