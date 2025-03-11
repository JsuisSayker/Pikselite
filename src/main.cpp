#include <editor/Editor.hpp>
#include <home/Home.hpp>
#include <memory>
#include <stdio.h>

int main()
{
    graphic::Position position;
    graphic::EventType event;
    std::shared_ptr<graphic::Graphic> graphic = std::make_shared<graphic::Graphic>();
    std::unique_ptr<Editor> editor = std::make_unique<Editor>(graphic);
    std::unique_ptr<Home> home = std::make_unique<Home>(graphic);

    home->run();
    return editor->run();
}
