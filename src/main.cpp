/**
 * @file main.cpp
 * @brief Entry point for the Pixel Engine application, initializing the core engine and starting
 * the main application loop. This file is part of the Pixel Engine project, which simulates
 * pixel-based physics and interactions.
 */
#define SDL_MAIN_HANDLED

#include <engine/core.hpp>
#include <windows.h>

/**
 * @brief The main function initializes the Core engine with the specified window dimensions and
 * starts the main application loop. The Core engine will manage the overall application state,
 * including rendering, user input, and game logic updates. The main function serves as the entry
 * point for the application, and will return 0 upon successful execution. Note: The Core engine is
 * responsible for creating the window, initializing the graphics and input systems, and managing
 * the main application loop, so the main function simply creates an instance of the Core class and
 * calls its run method to start the application.
 * @return An integer value of 0 indicating successful execution of the application.
 */
int main()
{
    SetDllDirectoryA(".\\dlls");
    engine::Core app(WINDOW_WIDTH, WINDOW_HEIGHT);
    app.run();
    return 0;
}
