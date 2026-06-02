/**
 * @file main.cpp
 * @brief Entry point for the Pixel Engine application, initializing the core engine and starting
 * the main application loop. This file is part of the Pixel Engine project, which simulates
 * pixel-based physics and interactions.
 */
#define SDL_MAIN_HANDLED

#include <engine/core.hpp>
#include <iostream>

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
    std::cerr << "[LOG] Engine starting" << std::endl;
    try
    {
        engine::Core app(WINDOW_WIDTH, WINDOW_HEIGHT);
        std::cerr << "[LOG] Core constructed" << std::endl;
        app.run();
        std::cerr << "[LOG] App finished normally" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[FATAL] Uncaught exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "[FATAL] Uncaught unknown exception" << std::endl;
        return 1;
    }
    return 0;
}
