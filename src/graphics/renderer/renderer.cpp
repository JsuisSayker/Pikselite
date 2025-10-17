#include <graphics/renderer/renderer.hpp>

const char* vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;
uniform vec2 uResolution;
uniform float uPointSize; // new uniform

void main() {
    vec2 pos = (aPos / uResolution) * 2.0 - 1.0;
    pos.y = -pos.y;
    gl_Position = vec4(pos, 0.0, 1.0);
    gl_PointSize = uPointSize; // use uniform
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

namespace graphics {
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
    }


    Renderer::~Renderer()
    {
        glDeleteProgram(_shader);
        glDeleteBuffers(1, &_vbo);
        glDeleteVertexArrays(1, &_vao);
    }

    void checkShaderCompile(GLuint shader, const char* name) {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "Shader compilation failed (" << name << "):\n" << infoLog << std::endl;
        }
    }

    void checkProgramLink(GLuint program) {
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
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

    void Renderer::clear() {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer::present(SDL_Window* window) {
        SDL_GL_SwapWindow(window);
    }

    void Renderer::drawPixels(const std::vector<Pixel>& pixels, float pixelSize) {
        if (pixels.empty()) return;

        int width, height;
        SDL_GetWindowSize(_window, &width, &height);

        glUseProgram(_shader);
        // set uniforms
        GLint resLoc = glGetUniformLocation(_shader, "uResolution");
        if (resLoc != -1) glUniform2f(resLoc, (float)width, (float)height);

        GLint sizeLoc = glGetUniformLocation(_shader, "uPointSize");
        if (sizeLoc != -1) glUniform1f(sizeLoc, pixelSize);

        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, pixels.size() * sizeof(Pixel), pixels.data(), GL_DYNAMIC_DRAW);

        // Attributes layout
        // position (location = 0) : vec2 (x,y)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Pixel), (void*)0);
        glEnableVertexAttribArray(0);

        // color (location = 1) : vec3 (r,g,b)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Pixel), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // removed attribute location 2 (we now use a uniform for size)

        glDrawArrays(GL_POINTS, 0, (GLsizei)pixels.size());
        glBindVertexArray(0);
    }

    void Renderer::drawGrid(float cellSize, float r, float g, float b) {
        int width, height;
        SDL_GetWindowSize(_window, &width, &height);

        std::vector<Pixel> gridLines;

        // Vertical lines
        for (float x = 0; x <= width; x += cellSize) {
            for (float y = 0; y <= height; y += 1.0f) {
                gridLines.push_back({x, y, r, g, b});
            }
        }

        // Horizontal lines
        for (float y = 0; y <= height; y += cellSize) {
            for (float x = 0; x <= width; x += 1.0f) {
                gridLines.push_back({x, y, r, g, b});
            }
        }

        drawPixels(gridLines, 1.0f);
    }
}
