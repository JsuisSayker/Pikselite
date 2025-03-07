#include <graphic/Graphic.hpp>

namespace graphic
{
    Graphic::Graphic()
    {
        this->_window = SDL_CreateWindow("Pikselite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        this->_renderer = SDL_CreateRenderer(this->_window, -1, SDL_RENDERER_ACCELERATED);

        if (!this->_window || !this->_renderer)
            throw std::runtime_error("Error: SDL2 failed to initialize.");

        // set the window background color to white
        SDL_SetRenderDrawColor(this->_renderer, 255, 255, 255, 255);
        SDL_RenderClear(this->_renderer);
        SDL_RenderPresent(this->_renderer);
    }

    Graphic::~Graphic()
    {
        SDL_DestroyRenderer(this->_renderer);
        SDL_DestroyWindow(this->_window);
        SDL_Quit();
    }

    void Graphic::updateWindow()
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
        SDL_RenderPresent(_renderer);
    }

    void Graphic::clearWindow()
    {
        SDL_SetRenderDrawColor(this->_renderer, 255, 255, 255, 255);
        SDL_RenderClear(this->_renderer);
    }

    EventType Graphic::checkEvent()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            // if the user closes the window
            if (event.type == SDL_QUIT)
            {
                this->_windowOpen = false;
                return EventType::WINDOW_CLOSE;
            }

            // if the user clicks mouse button
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                    return EventType::MOUSE_CLICK_LEFT;
            }

            // if the user moves the mouse
            if (event.type == SDL_MOUSEMOTION)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                    return EventType::MOUSE_DRAG_LEFT;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    return EventType::MOUSE_DRAG_RIGHT;
                return EventType::MOUSE_MOVE;
            }

            // if the user presses a key
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_a:
                    return EventType::KEY_A;
                case SDLK_b:
                    return EventType::KEY_B;
                case SDLK_c:
                    return EventType::KEY_C;
                case SDLK_d:
                    return EventType::KEY_D;
                case SDLK_e:
                    return EventType::KEY_E;
                case SDLK_f:
                    return EventType::KEY_F;
                case SDLK_g:
                    return EventType::KEY_G;
                case SDLK_h:
                    return EventType::KEY_H;
                case SDLK_i:
                    return EventType::KEY_I;
                case SDLK_j:
                    return EventType::KEY_J;
                case SDLK_k:
                    return EventType::KEY_K;
                case SDLK_l:
                    return EventType::KEY_L;
                case SDLK_m:
                    return EventType::KEY_M;
                case SDLK_n:
                    return EventType::KEY_N;
                case SDLK_o:
                    return EventType::KEY_O;
                case SDLK_p:
                    return EventType::KEY_P;
                case SDLK_q:
                    return EventType::KEY_Q;
                case SDLK_r:
                    return EventType::KEY_R;
                case SDLK_s:
                    return EventType::KEY_S;
                case SDLK_t:
                    return EventType::KEY_T;
                case SDLK_u:
                    return EventType::KEY_U;
                case SDLK_v:
                    return EventType::KEY_V;
                case SDLK_w:
                    return EventType::KEY_W;
                case SDLK_x:
                    return EventType::KEY_X;
                case SDLK_y:
                    return EventType::KEY_Y;
                case SDLK_z:
                    return EventType::KEY_Z;
                case SDLK_UP:
                    return EventType::KEY_ARROW_UP;
                case SDLK_DOWN:
                    return EventType::KEY_ARROW_DOWN;
                case SDLK_LEFT:
                    return EventType::KEY_ARROW_LEFT;
                case SDLK_RIGHT:
                    return EventType::KEY_ARROW_RIGHT;
                default:
                    return EventType::KEYBOARD_PRESS;
                }
            }
        }
        return EventType::NONE;
    }

    Position Graphic::getPosition()
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return Position{static_cast<float>(x), static_cast<float>(y)};
    }

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

}
