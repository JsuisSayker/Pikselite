#include <graphic/Graphic.hpp>

namespace graphic
{
    void Graphic::drawPixel(Pixel pixel, Camera camera)
    {
        float zoom = static_cast<float>(camera.zoom);
        float leftEdge = camera.position.x - (WINDOW_WIDTH / (2.0f * zoom));
        float topEdge = camera.position.y - (WINDOW_HEIGHT / (2.0f * zoom));

        // Calculate screen coordinates based on world position
        float screenX = (pixel.position.x - leftEdge) * zoom;
        float screenY = (pixel.position.y - topEdge) * zoom;

        int cellSize = static_cast<int>(zoom); // assuming each cell is 'zoom' pixels wide
        // Offset by half the cell size to center the rectangle at the grid point
        SDL_Rect rect = {
            static_cast<int>(std::round(screenX - cellSize / 2.0f)),
            static_cast<int>(std::round(screenY - cellSize / 2.0f)),
            cellSize,
            cellSize};

        SDL_SetRenderDrawColor(_renderer, pixel.color.r, pixel.color.g, pixel.color.b, pixel.color.a);
        SDL_RenderFillRect(_renderer, &rect);
    }

    void Graphic::drawGrid(Camera camera)
    {
        float zoom = static_cast<float>(camera.zoom);
        float leftEdge = camera.position.x - (WINDOW_WIDTH / (2.0f * zoom));
        float rightEdge = camera.position.x + (WINDOW_WIDTH / (2.0f * zoom));
        float topEdge = camera.position.y - (WINDOW_HEIGHT / (2.0f * zoom));
        float bottomEdge = camera.position.y + (WINDOW_HEIGHT / (2.0f * zoom));

        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);

        // Calculate first vertical grid line (smallest line position >= leftEdge)
        float firstVertical = std::ceil(leftEdge - 0.5f) + 0.5f;
        for (float x = firstVertical; x < rightEdge; x += 1.0f)
        {
            float screenX = (x - leftEdge) * zoom;
            SDL_RenderDrawLine(_renderer, static_cast<int>(std::round(screenX)), 0,
                               static_cast<int>(std::round(screenX)), WINDOW_HEIGHT);
        }

        // Calculate first horizontal grid line (smallest line position >= topEdge)
        float firstHorizontal = std::ceil(topEdge - 0.5f) + 0.5f;
        for (float y = firstHorizontal; y < bottomEdge; y += 1.0f)
        {
            float screenY = (y - topEdge) * zoom;
            SDL_RenderDrawLine(_renderer, 0, static_cast<int>(std::round(screenY)),
                               WINDOW_WIDTH, static_cast<int>(std::round(screenY)));
        }
    }

    void Graphic::drawPixels(Camera camera)
    {
        float zoomFactor = static_cast<float>(camera.zoom);
        float leftEdge = camera.position.x - (WINDOW_WIDTH / (2.0f * zoomFactor));
        float rightEdge = camera.position.x + (WINDOW_WIDTH / (2.0f * zoomFactor));
        float topEdge = camera.position.y - (WINDOW_HEIGHT / (2.0f * zoomFactor));
        float bottomEdge = camera.position.y + (WINDOW_HEIGHT / (2.0f * zoomFactor));

        for (const Pixel &pixel : _pixels)
        {
            if (pixel.position.x >= leftEdge && pixel.position.x < rightEdge &&
                pixel.position.y >= topEdge && pixel.position.y < bottomEdge)
            {
                drawPixel(pixel, camera);
            }
        }
    }

    void Graphic::drawRectangle(Rectangle rectangle)
    {
        // dont use camera
        SDL_Rect rect = {
            static_cast<int>(rectangle.position.x),
            static_cast<int>(rectangle.position.y),
            rectangle.width,
            rectangle.height};

        SDL_SetRenderDrawColor(_renderer, rectangle.color.r, rectangle.color.g, rectangle.color.b, rectangle.color.a);
        SDL_RenderFillRect(_renderer, &rect);
    }
}
