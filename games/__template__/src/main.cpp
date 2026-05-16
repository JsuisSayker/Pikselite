#define SDL_MAIN_HANDLED
#include <filesystem>
#include <game/Game.hpp>

int main(int argc, char* argv[])
{
    std::filesystem::current_path(std::filesystem::absolute(argv[0]).parent_path());
    engine::Game game({{WINDOW_WIDTH}}, {{WINDOW_HEIGHT}}, "{{GAME_TITLE}}");
    if (!game.loadScene("assets/scene.scene"))
        return 1;
    game.run();
    return 0;
}
