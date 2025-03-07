/*
** EPITECH PROJECT, 2025
** SDL2_POC
** File description:
** main
*/

#include <graphic/Graphic.hpp>
#include <memory>
#include <stdio.h>

int main()
{
    graphic::Position position;
    graphic::EventType event;
    std::unique_ptr<graphic::Graphic> graphic = std::make_unique<graphic::Graphic>();

    while (graphic->_windowOpen)
    {
        event = graphic->checkEvent();

        if (event == graphic::EventType::WINDOW_CLOSE)
            break;

        if (event == graphic::EventType::MOUSE_CLICK_LEFT || event == graphic::EventType::MOUSE_DRAG_LEFT)
        {
            position = graphic->getPosition();
            float zoom = static_cast<float>(graphic->camera.zoom);
            position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + graphic->camera.position.x;
            position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + graphic->camera.position.y;

            // Snap to grid (integer grid)
            position.x = std::round(position.x);
            position.y = std::round(position.y);

            graphic->_pixels.push_back(graphic::Pixel{position, {255, 0, 0, 255}});
        }

        if (event == graphic::EventType::KEY_ARROW_UP)
        {
            graphic->camera.position.y -= 10;
        }
        if (event == graphic::EventType::KEY_ARROW_DOWN)
        {
            graphic->camera.position.y += 10;
        }
        if (event == graphic::EventType::KEY_ARROW_LEFT)
        {
            graphic->camera.position.x -= 10;
        }
        if (event == graphic::EventType::KEY_ARROW_RIGHT)
        {
            graphic->camera.position.x += 10;
        }
        if (event == graphic::EventType::KEY_I)
        {
            graphic->camera.zoom += 1;
        }
        if (event == graphic::EventType::KEY_O)
        {
            graphic->camera.zoom -= 1;
            if (graphic->camera.zoom < 1)
                graphic->camera.zoom = 1;
        }
        graphic->clearWindow();
        graphic->updateWindow();
    }
    return 0;
}
