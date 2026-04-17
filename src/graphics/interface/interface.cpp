#include <graphics/interface/interface.hpp>

namespace graphics {
    Interface::Interface(int width, int height)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Error SDL_Init: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to initialize SDL");
        }

        SDL_DisplayMode displayMode;
        if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
            width = displayMode.w;
            height = displayMode.h;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        _window = SDL_CreateWindow(
            "Pikselite Engine",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );

        if (!_window) {
            std::cerr << "Error SDL_CreateWindow: " << SDL_GetError() << std::endl;
            SDL_Quit();
            throw std::runtime_error("Failed to create window");
        }

        SDL_SetWindowIcon(_window, SDL_LoadBMP("assets/icon.bmp"));

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
        if (_glContext) {
            SDL_GL_DeleteContext(_glContext);
        }
        if (_window) {
            SDL_DestroyWindow(_window);
        }
        SDL_Quit();
    }

    InputEvent Interface::pollEvent()
    {
        SDL_Event event;
        InputEvent result;
        result.type = NO_EVENT;
        result.windowID = 0;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_DROPFILE)
            {
                result.type = FILE_DROPPED;
                result.windowID = event.drop.windowID;
                if (event.drop.file)
                {
                    result.droppedFilePath = event.drop.file;
                    SDL_free(event.drop.file);
                }
                return result;
            }

            ImGuiIO &io = ImGui::GetIO();
            if (io.WantCaptureKeyboard || io.WantCaptureMouse)
                continue;
            switch (event.type) {
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    std::cout << "Window close event for window ID: " << event.window.windowID << std::endl;
                    result.type = WINDOW_CLOSE;
                    result.windowID = event.window.windowID;
                    return result;
                }
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    glViewport(0, 0, event.window.data1, event.window.data2);
                }
                break;
            case SDL_KEYDOWN:
                result.windowID = event.key.windowID;
                switch (event.key.keysym.sym) {
                case SDLK_w: result.type = KEY_W; return result;
                case SDLK_a: result.type = KEY_A; return result;
                case SDLK_s: result.type = KEY_S; return result;
                case SDLK_d: result.type = KEY_D; return result;
                case SDLK_i: result.type = KEY_I; return result;
                case SDLK_k: result.type = KEY_K; return result;
                case SDLK_l: result.type = KEY_L; return result;
                case SDLK_o: result.type = KEY_O; return result;
                case SDLK_TAB: result.type = KEY_TAB; return result;
                case SDLK_F5: result.type = KEY_F5; return result;
                case SDLK_ESCAPE: result.type = KEY_ESCAPE; return result;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    result.type = MOUSE_LEFT_CLICK;
                    result.windowID = event.button.windowID;
                    return result;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    result.type = MOUSE_RIGHT_CLICK;
                    result.windowID = event.button.windowID;
                    return result;
                }
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.state & SDL_BUTTON_LMASK) {
                    result.type = MOUSE_LEFT_DRAG;
                    result.windowID = event.motion.windowID;
                    return result;
                }
                if (event.motion.state & SDL_BUTTON_RMASK) {
                    result.type = MOUSE_RIGHT_DRAG;
                    result.windowID = event.motion.windowID;
                    return result;
                }
                break;
            case SDL_QUIT:
                result.type = QUIT;
                result.windowID = 0;
                return result;
            }
        }
        return result;
    }

    glm::vec2 Interface::getMousePosition() const
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return glm::vec2(static_cast<float>(x), static_cast<float>(y));
    }
}
