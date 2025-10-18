#include <graphics/interface/interface.hpp>

namespace graphics {
    Interface::Interface(int width, int height)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Error SDL_Init: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to initialize SDL");
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        _window = SDL_CreateWindow(
            "SDL2 + OpenGL test",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
        );

        if (!_window) {
            std::cerr << "Error SDL_CreateWindow: " << SDL_GetError() << std::endl;
            SDL_Quit();
            throw std::runtime_error("Failed to create window");
        }

        _glContext = SDL_GL_CreateContext(_window);
        if (!_glContext) {
            std::cerr << "Error SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(_window);
            SDL_Quit();
            throw std::runtime_error("Failed to create OpenGL context");
        }
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            std::cerr << "Error GLEW: " << glewGetErrorString(glewError) << std::endl;
            SDL_GL_DeleteContext(_glContext);
            SDL_DestroyWindow(_window);
            SDL_Quit();
            throw std::runtime_error("Failed to initialize GLEW");
        }
        std::cout << "OpenGL version : " << glGetString(GL_VERSION) << std::endl;
    }

    Interface::~Interface()
    {
        if (_window) {
            SDL_DestroyWindow(_window);
        }
        SDL_Quit();
    }

    InputEventType Interface::pollEvent()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return QUIT;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    return MOUSE_LEFT_CLICK;
                }
            }
        }
        return NO_EVENT;
    }

    glm::vec2 Interface::getMousePosition() const
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return glm::vec2(static_cast<float>(x), static_cast<float>(y));
    }
}
