/**
 * @file interface.hpp
 * @brief Declares the Interface class for managing the graphical interface and user input.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and
 * interactions.
 */
#pragma once

#include <graphics/graphicsEnum.hpp>
#include <imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <iostream>

/**
 * @brief The graphics namespace contains classes and functions related to rendering and user
 * interface management. The Interface class provides methods for creating a window, handling user
 * input, and managing the OpenGL context.
 */
namespace graphics
{
    class Interface
    {
      public:
        /**
         * @brief Constructs the Interface, creating a window and initializing the OpenGL context.
         * @param width The width of the window in pixels.
         * @param height The height of the window in pixels.
         */
        Interface(int width, int height);
        /**
         * @brief Destructs the Interface, cleaning up resources and closing the window.
         */
        ~Interface();

        // Accessor methods for the SDL window, OpenGL context, and window ID
        SDL_Window* getWindow() const
        {
            return _window;
        }
        SDL_GLContext getGLContext() const
        {
            return _glContext;
        }
        uint32_t getWindowID() const
        {
            return SDL_GetWindowID(_window);
        }

        /**
         * @brief Polls for user input events, such as keyboard and mouse events, and returns the
         * next event in the queue.
         * @return An SDL_Event representing the next user input event, or an empty event if no
         * events are available.
         */
        InputEvent pollEvent();

        /**
         * @brief Retrieves the current position of the mouse cursor relative to the window.
         * @return A glm::vec2 containing the x and y coordinates of the mouse cursor.
         */
        glm::vec2 getMousePosition() const;

      private:
        // SDL window and OpenGL context for rendering
        SDL_Window* _window;
        SDL_GLContext _glContext;
    };
} // namespace graphics
