#include <editor/Editor.hpp>

Editor::Editor(std::shared_ptr<graphic::Graphic> graphic) : _graphic(graphic)
{
}

Editor::~Editor()
{
}

void Editor::run()
{
    graphic::Position position;
    graphic::EventType event;

    while (_graphic->_windowOpen)
    {
        event = _graphic->checkEvent();

        if (event == graphic::EventType::WINDOW_CLOSE)
            break;

        if (event == graphic::EventType::MOUSE_CLICK_LEFT || event == graphic::EventType::MOUSE_DRAG_LEFT)
        {
            position = _graphic->getPosition();
            float zoom = static_cast<float>(_graphic->camera.zoom);
            position.x = (position.x - WINDOW_WIDTH / 2.0f) / zoom + _graphic->camera.position.x;
            position.y = (position.y - WINDOW_HEIGHT / 2.0f) / zoom + _graphic->camera.position.y;

            // Snap to grid (integer grid)
            position.x = std::round(position.x);
            position.y = std::round(position.y);

            _graphic->_pixels.push_back(graphic::Pixel{position, {255, 0, 0, 255}});
        }

        if (event == graphic::EventType::KEY_ARROW_UP)
        {
            _graphic->camera.position.y -= 10;
        }
        if (event == graphic::EventType::KEY_ARROW_DOWN)
        {
            _graphic->camera.position.y += 10;
        }
        if (event == graphic::EventType::KEY_ARROW_LEFT)
        {
            _graphic->camera.position.x -= 10;
        }
        if (event == graphic::EventType::KEY_ARROW_RIGHT)
        {
            _graphic->camera.position.x += 10;
        }
        if (event == graphic::EventType::KEY_I)
        {
            _graphic->camera.zoom += 1;
        }
        if (event == graphic::EventType::KEY_O)
        {
            _graphic->camera.zoom -= 1;
            if (_graphic->camera.zoom < 4)
                _graphic->camera.zoom = 4;
        }
        _graphic->clearWindow();
        _graphic->updateWindow();
    }
}