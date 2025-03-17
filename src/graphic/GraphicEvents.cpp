#include <graphic/Graphic.hpp>

namespace graphic
{
    EventType Graphic::checkEvent()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            // if the user scrolls the mouse wheel
            if (event.type == SDL_MOUSEWHEEL)
            {
                if (event.wheel.y > 0)
                    return EventType::MOUSE_WHEEL_UP;
                if (event.wheel.y < 0)
                    return EventType::MOUSE_WHEEL_DOWN;
            }

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                if (event.window.windowID == SDL_GetWindowID(this->_window))
                {
                    this->_windowOpen = false;
                    return EventType::WINDOW_CLOSE;
                }
            }

            if (showInterface)
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                ImGuiIO &io = ImGui::GetIO();
                if (io.WantCaptureMouse)
                    return EventType::NONE;
            }

            // if the user clicks mouse button
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                    return EventType::MOUSE_CLICK_LEFT;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    return EventType::MOUSE_CLICK_RIGHT;
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
                case SDLK_TAB:
                    return EventType::KEY_TAB;
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
}
