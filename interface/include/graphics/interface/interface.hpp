#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>

namespace graphics {
    enum InputEventType {
        MOUSE_LEFT_CLICK,
        QUIT,
        NO_EVENT,
    };

    class Interface {
        public:
            Interface(int width, int height);
            ~Interface();

            SDL_Window* getWindow() const { return _window; }
            SDL_GLContext getGLContext() const { return _glContext; }

            InputEventType pollEvent();

        private:
            SDL_Window* _window;
            SDL_GLContext _glContext;
    };
}
