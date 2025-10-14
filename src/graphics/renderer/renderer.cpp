#include <graphics/renderer/renderer.hpp>


const char* vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aSize;

out vec3 vColor;
uniform vec2 uResolution;

void main() {
    vec2 pos = (aPos / uResolution) * 2.0 - 1.0;
    pos.y = -pos.y;
    gl_Position = vec4(pos, 0.0, 1.0);
    gl_PointSize = aSize;
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
        glViewport(0, 0, 800, 600);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

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

    void Renderer::drawPixels(const std::vector<Pixel>& pixels) {
        if (pixels.empty()) return;

        int width, height;
        SDL_GetWindowSize(_window, &width, &height);

        glUseProgram(_shader);
        glUniform2f(glGetUniformLocation(_shader, "uResolution"), (float)width, (float)height);

        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, pixels.size() * sizeof(Pixel), pixels.data(), GL_DYNAMIC_DRAW);

        // Attributes layout
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Pixel), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Pixel), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Pixel), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glDrawArrays(GL_POINTS, 0, (GLsizei)pixels.size());
        glBindVertexArray(0);
    }
}
