#include <home/Home.hpp>

Home::Home(std::shared_ptr<graphic::Graphic> graphic) : _graphic(graphic)
{
}

Home::~Home()
{
}

int Home::run()
{
    graphic::EventType event;
    _graphic->showHome = true;

    while (_graphic->_windowOpen)
    {
        event = _graphic->checkEvent();

        if (event == graphic::EventType::WINDOW_CLOSE || _graphic->showHome == false)
            return 0;

        _graphic->clearWindow();

        _graphic->drawInterface();

        _graphic->updateWindow();
    }
    _graphic->showHome = false;
    return 0;
}