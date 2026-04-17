/**
 * @file renderer.hpp
 * @brief Declares the Renderer class for managing rendering of pixels and sprites using OpenGL.
 * This file is part of the Pixel Engine project, which simulates pixel-based physics and interactions.
 */
#pragma once

#include <graphics/graphicsEnum.hpp>
#include <engine/pixels/pixelEnum.hpp>
#include <graphics/renderer/camera.hpp>
#include <box2d/box2d.h>

#include <vector>
#include <iostream>

/**
 * @brief The graphics namespace contains classes and functions related to rendering and user interface management.
 * The Renderer class provides methods for rendering pixels and sprites, managing OpenGL resources, and drawing with camera transformations.
 */
namespace graphics {
    class Renderer {
    public:
        /**
         * @brief Constructs the Renderer, initializing OpenGL resources for rendering pixels and sprites.
         * @param window The SDL_Window to render to.
         * @param glContext The OpenGL context associated with the window.
         * Note: The Renderer does not take ownership of the window or OpenGL context, and assumes they are valid for the lifetime of the Renderer.
         */
        Renderer(SDL_Window* window, SDL_GLContext glContext);
        /**
         * @brief Destructs the Renderer, cleaning up OpenGL resources.
         * Note: The Renderer does not destroy the SDL_Window or OpenGL context, as it does not own them.
         */
        ~Renderer();

        /**
         * @brief Clears the screen and presents the rendered frame to the window.
         * @param window The SDL_Window to present the rendered frame to.
         * Note: This method should be called after all drawing calls for the frame are completed, and before the next frame's drawing calls begin.
         */
        void clear();
        /**
         * @brief Presents the rendered frame to the window.
         * @param window The SDL_Window to present the rendered frame to.
         * Note: This method should be called after all drawing calls for the frame are completed, and before the next frame's drawing calls begin.
         */
        void present(SDL_Window* window);
        /**
         * @brief Draws a collection of pixels to the screen, using the specified pixel size for rendering.
         * @param pixels A vector of Pixel structs containing the position and color of each pixel to be rendered.
         * @param pixelSize The size of each pixel when rendered on the screen, in pixels. Default is PIXEL_SIZE.
         * Note: The position of each pixel is expected to be in world coordinates, and will be transformed to screen coordinates based on the current camera view if using drawPixelsWCamera.
         */
        void drawPixelsOverlay(const std::vector<Pixel>& pixels, float pixelSize = PIXEL_SIZE);
        /**
         * @brief Draws a collection of pixels to the screen, applying transformations based on the provided camera's view and zoom level.
         * @param pixels A vector of Pixel structs containing the position and color of each pixel to
         * be rendered. The position of each pixel is expected to be in world coordinates, and will be transformed to screen coordinates based on the camera's position and zoom level.
         * @param camera A Camera2D object representing the current view and zoom level for rendering
         * @param pixelSize The size of each pixel when rendered on the screen, in pixels. Default is PIXEL_SIZE.
         * Note: This method applies the camera's view-projection transformation to the pixel positions, allowing for panning and zooming effects. The pixel size will be scaled based on the camera's zoom level to maintain consistent visual size on the screen.
         */
        void drawPixelsWCamera(const std::vector<Pixel>& pixels, const Camera2D &camera, float pixelSize = PIXEL_SIZE);
        /**
         * @brief Draws a grid overlay on the screen, with lines spaced according to the specified cell size and colored based on the provided color.
         * @param camera A Camera2D object representing the current view and zoom level for rendering the grid. The grid will be transformed based on the camera's position and zoom level.
         * @param cellSize The size of each cell in the grid, in world coordinates. The grid lines will be spaced according to this cell size, and will be transformed based on the camera's zoom level to maintain consistent spacing on the screen.
         * @param color A glm::vec3 representing the RGB color of the grid lines. Each component should be in the range [0.0, 1.0].
         * Note: This method applies the camera's view-projection transformation to the grid lines, allowing for panning and zooming effects. The grid will be drawn as a series of lines spaced according to the cell size, and will cover the visible area of the screen based on the camera's view.
         */
        void drawGrid(const Camera2D& camera, float cellSize, glm::vec3 color);
        void drawSegments(const std::vector<LineVertex>& segments, const Camera2D& camera);
        void drawBox2DDebug(b2WorldId worldId, const std::vector<b2BodyId>& bodies, const Camera2D& camera, float pixelsPerMeter, glm::vec3 color);

        /**
         * @brief Loads a texture from the specified file path and returns its OpenGL texture ID.
         * @param filePath The file path to the texture image to be loaded. The image
         * should be in a format supported by the image loading library (e.g., PNG, JPEG).
         * @return The OpenGL texture ID of the loaded texture, or 0 if loading
         */
        GLuint loadTexture(const std::string& filePath);
        /**
         * @brief Draws a sprite to the screen, applying transformations based on the provided camera's view and zoom level.
         * @param sprite A Sprite2D struct containing the texture ID, position, size, and color of the sprite to be rendered. The position is expected to be in world coordinates, and will be transformed to screen coordinates based on the camera's position and zoom level.
         * @param camera A Camera2D object representing the current view and zoom level for rendering the sprite. The sprite will be transformed based on the camera's position and zoom level.
         * Note: This method applies the camera's view-projection transformation to the sprite's position, allowing for panning and zooming effects. The sprite will be drawn using the specified texture, and its size will be scaled based on the camera's zoom level to maintain consistent visual size on the screen.
         */
        void drawSprite(const Sprite2D& sprite, const Camera2D& camera);
        /**
         * @brief Unloads a texture from OpenGL, freeing its resources.
         * @param textureID The OpenGL texture ID of the texture to be unloaded. This should be a valid texture ID that was previously loaded using loadTexture.
         * Note: After calling this method, the specified texture ID will no longer be valid and should not be used for rendering. Any sprites or pixels using this texture will need to be updated to use a different texture or removed from rendering.
         */
        void unloadTexture(GLuint textureID);

    private:
        SDL_Window* _window;
        SDL_GLContext _glContext;
        // Pixel shader
        GLuint _vao, _vbo, _shader;

        // Sprite shader
        GLuint _spriteVao, _spriteVbo, _spriteShader;
        /**
         * @brief Initializes the OpenGL shader program for rendering pixels, compiling the vertex and fragment shaders and linking them into a program. The shader will be used for rendering pixels in the drawPixelsOverlay and drawPixelsWCamera methods.
         * Note: This method is called during the construction of the Renderer, and assumes that a valid OpenGL context is available. If shader compilation or linking fails, an error message will be printed to the console.
         */
        void initShader();
        /**
         * @brief Initializes the OpenGL shader program for rendering sprites, compiling the vertex and fragment shaders and linking them into a program. The shader will be used for rendering sprites in the drawSprite method.
         * Note: This method is called during the construction of the Renderer, and assumes that a valid OpenGL context is available. If shader compilation or linking fails, an error message will be printed to the console.
         */
        void initSpriteShader();
    };
}
