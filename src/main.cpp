#define SDL_MAIN_HANDLED
#include <engine/core.hpp>

int main()
{
    engine::Core app(WINDOW_WIDTH, WINDOW_HEIGHT);
    app.run();
    return 0;
}
