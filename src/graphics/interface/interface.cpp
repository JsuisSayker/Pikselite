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
            ImGui_ImplSDL2_ProcessEvent(&event);
            ImGuiIO &io = ImGui::GetIO();
            if (io.WantCaptureKeyboard || io.WantCaptureMouse)
                continue;
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_w: return KEY_W;
                case SDLK_a: return KEY_A;
                case SDLK_s: return KEY_S;
                case SDLK_d: return KEY_D;
                case SDLK_i: return KEY_I;
                case SDLK_k: return KEY_K;
                case SDLK_l: return KEY_L;
                case SDLK_o: return KEY_O;
                case SDLK_TAB: return KEY_TAB;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    return MOUSE_LEFT_CLICK;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    return MOUSE_RIGHT_CLICK;
                }
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.state & SDL_BUTTON_LMASK) {
                    return MOUSE_LEFT_DRAG;
                }
                if (event.motion.state & SDL_BUTTON_RMASK) {
                    return MOUSE_RIGHT_DRAG;
                }
                break;
            case SDL_QUIT:
                return QUIT;
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
