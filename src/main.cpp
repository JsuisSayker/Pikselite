#define SDL_MAIN_HANDLED
#include <engine/core.hpp>

int main()
{
    engine::Core app(1280, 720);
    app.run();
    return 0;
}
